using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Threading;

namespace DataStructures
{
    /// <summary>
    /// A concurrent rolling dictionary acting as an ALRU cache. (*)
    ///
    /// Until a specified capacity threshold is reached, it is nearly as
    /// efficient as a plain ConcurrentDictionary.  It is lock-free in a
    /// better-to-spin-than-to-lock way.
    ///
    /// (*) The evictions are actually done on the _almost_ least recently used items.
    /// </summary>
    public class LRUCache<TKey, TValue>
    {
        private const int DefaultConcurrency = 4;
        private const int DefaultCapacity = 31;
        private const double DefaultThresholdRatio = 0.90; // 90% of the capacity

        private readonly LinkedList<KeyValuePair<TKey, TValue>> _lru;
        private readonly ConcurrentDictionary<TKey, LinkedListNode<KeyValuePair<TKey, TValue>>> _dico;

        private readonly int _capacity;
        private readonly int _lruThreshold;

        #region SpinLock
        /// <summary>
        /// A custom lightweight spinlock. Not really reliable so don't mess with it.
        /// (There is no ConcurrentLinkedList so...)
        /// (Yeah, it's a pity Microsoft keep the all good low-level internals for them only.)
        /// (And Yeah, Monitor.TryEnter() sucks.)
        /// </summary>
        private struct SpinLock
        {
            private int _lock;

            public bool Lock(out bool lockTaken)
            {
                // while locked, spin (it will eventually yield)
                SpinWait sw = new SpinWait();
                while (Interlocked.CompareExchange(ref _lock, 1, 0) != 0) sw.SpinOnce();
                return (lockTaken = true);
            }

            public bool TryLock(out bool lockTaken)
            {
                // try for immediate success
                if (Interlocked.CompareExchange(ref _lock, 1, 0) == 0) return (lockTaken = true);
                // otherwise spin a bit (this is like one SpinOnce)
                Thread.SpinWait(4);
                // and try one more time
                return (lockTaken = (Interlocked.CompareExchange(ref _lock, 1, 0) == 0));
            }

            public bool UnLock(ref bool lockTaken)
            {
                if (lockTaken) Interlocked.Exchange(ref _lock, 0);
                return (lockTaken = false);
            }
        }
        SpinLock _spinlock;
        #endregion

        #region Metrics
        private long _nbHit;
        private long _nbMiss;
        private long _nbUpdate;
        private long _nbEviction;

        /// <summary/>
        public int Capacity { get { return _capacity; } }
        /// <summary/>
        public int Count { get { return _lru.Count; } }
        /// <summary/>
        public long NbHit { get { return _nbHit; } }
        /// <summary/>
        public long NbMiss { get { return _nbMiss; } }
        /// <summary/>
        public long NbUpdate { get { return _nbUpdate; } }
        /// <summary/>
        public long NbEviction { get { return _nbEviction; } } 
        #endregion

        /// <summary/>
        public LRUCache(int capacity = DefaultCapacity, int concurrency = DefaultConcurrency, double thresholdRatio = DefaultThresholdRatio)
        {
            _capacity = capacity;
            _spinlock = new SpinLock();

            // let's try to set a sensible threshold: 1 <= _lruThreshold <= _capacity
            if (thresholdRatio < 0 || thresholdRatio > 1) throw new ArgumentOutOfRangeException("thresholdRatio");
            _lruThreshold = Math.Max(1, Math.Min((int)(thresholdRatio * capacity), capacity));

            _lru = new LinkedList<KeyValuePair<TKey, TValue>>();
            _dico = new ConcurrentDictionary<TKey, LinkedListNode<KeyValuePair<TKey, TValue>>>(concurrency * Environment.ProcessorCount, capacity);
        }

        /// <summary/>
        public TValue Get(TKey key)
        {
            LinkedListNode<KeyValuePair<TKey, TValue>> item;
            return (_capacity > 0 && TryGet(key, out item) ? item.Value.Value : default(TValue));
        }

        /// <summary/>
        public void Set(TKey key, TValue val)
        {
            if (_capacity == 0) return;

            bool lockTaken = false;
            LinkedListNode<KeyValuePair<TKey, TValue>> item;
            if (TryGet(key, out item)) // will also try to update the LRU
            {
                // Update
                try
                {
                    if (_spinlock.Lock(out lockTaken))
                    {
                        item.Value = new KeyValuePair<TKey, TValue>(key, val);
                        _spinlock.UnLock(ref lockTaken);

                        UpdateLRU(item);
                        Interlocked.Increment(ref _nbUpdate);
                    }
                }
                finally
                {
                    _spinlock.UnLock(ref lockTaken);
                }
            }
            else
            {
                // Addition (push back)
                var newNode = new LinkedListNode<KeyValuePair<TKey, TValue>>(new KeyValuePair<TKey, TValue>(key, val));
                if (_dico.TryAdd(key, newNode))
                {
                    try
                    {
                        if (_spinlock.Lock(out lockTaken))
                        {
                            if (newNode.List == null) _lru.AddLast(newNode);
                            _spinlock.UnLock(ref lockTaken);

                            Interlocked.Increment(ref _nbUpdate);
                        }
                    }
                    finally
                    {
                        _spinlock.UnLock(ref lockTaken);
                    }
                }

                // Evictions (pop front)
                while (_lru.Count > _capacity)
                {
                    try
                    {
                        if (_spinlock.Lock(out lockTaken))
                        {
                            if (_lru.Count <= _capacity) break; // double checked LRU :-)
                            var oldKey = _lru.First.Value.Key;
                            _lru.RemoveFirst();
                            _spinlock.UnLock(ref lockTaken);

                            LinkedListNode<KeyValuePair<TKey, TValue>> oldNode;
                            if (!_dico.TryRemove(oldKey, out oldNode))
                            {
                                if (_spinlock.Lock(out lockTaken))
                                {
                                    if (oldNode != null && oldNode.List != null) _lru.AddFirst(oldNode); // rollback
                                    _spinlock.UnLock(ref lockTaken);
                                }
                            } else {
                                Interlocked.Increment(ref _nbEviction);
                            }
                        }
                    }
                    finally
                    {
                        _spinlock.UnLock(ref lockTaken);
                    }
                }
            }            
        }

        /// <summary/>
        public bool Remove(TKey key)
        {
            if (_capacity == 0) return true;

            LinkedListNode<KeyValuePair<TKey, TValue>> item;
            if (_dico.TryRemove(key, out item))
            {
                bool lockTaken = false;
                try
                {
                    if (_spinlock.Lock(out lockTaken))
                    {
                        if (item != null && item.List != null) _lru.Remove(item.Value);
                        return true;
                    }
                }
                finally
                {
                    _spinlock.UnLock(ref lockTaken);
                }
            }
            return false;
        }

        /// <summary/>
        public bool Clear()
        {
            if (_capacity == 0) return true;

            bool lockTaken = false;
            try
            {
                if (_spinlock.Lock(out lockTaken))
                {
                    _dico.Clear();
                    _lru.Clear();
                    return true;
                }
            }
            finally
            {
                _spinlock.UnLock(ref lockTaken);
            }
            return false;
        }

        #region private methods
        /// <summary>
        /// The internal TryGet will also try to update the LRU cache but won't block.
        /// </summary>
        private bool TryGet(TKey key, out LinkedListNode<KeyValuePair<TKey, TValue>> item)
        {
            if (_dico.TryGetValue(key, out item))
            {
                UpdateLRU(item);
                Interlocked.Increment(ref _nbHit);
                return true;
            }

            item = null;
            Interlocked.Increment(ref _nbMiss);
            return false;
        }

        /// <summary>
        /// Try to maintain the LRU only if we crossed the threshold.
        /// </summary>
        private void UpdateLRU(LinkedListNode<KeyValuePair<TKey, TValue>> item)
        {
            if (_lru.Count >= _lruThreshold)
            {
                bool lockTaken = false;
                try
                {
                    _spinlock.TryLock(out lockTaken); // try once!
                    if (lockTaken && item != null && item.List != null)
                    {
                        _lru.Remove(item);
                        _lru.AddLast(item);
                    }
                }
                finally
                {
                    _spinlock.UnLock(ref lockTaken);
                }
            }
        }

        /// <summary>
        /// Bypass the LRU, mostly used for unit tests.
        /// </summary>
        public TValue RawGet(TKey key)
        {
            LinkedListNode<KeyValuePair<TKey, TValue>> item;
            return _dico.TryGetValue(key, out item) ? item.Value.Value : default(TValue);
        }
        #endregion
    }
}
