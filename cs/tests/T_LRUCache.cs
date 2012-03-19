using System;
using System.Threading.Tasks;
using NUnit.Framework;

namespace DataStructures
{
    [TestFixture]
    class T_LRUCache
    {
        [Test]
        public void SetGet()
        {
            var dico = new LRUCache<string, int>();

            dico.Set("the answer", 42);
            Assert.AreEqual(42, dico.RawGet("the answer"));
            Assert.AreEqual(1, dico.Count);
            Assert.AreEqual(1, dico.NbUpdate);
            Assert.AreEqual(0, dico.NbHit);
            Assert.AreEqual(1, dico.NbMiss);
            Assert.AreEqual(0, dico.NbEviction);

            int fortytwo = dico.Get("the answer");
            Assert.AreEqual(42, fortytwo);
            Assert.AreEqual(1, dico.Count);
            Assert.AreEqual(1, dico.NbUpdate);
            Assert.AreEqual(1, dico.NbHit);
            Assert.AreEqual(1, dico.NbMiss);
            Assert.AreEqual(0, dico.NbEviction);

            int fortythree = dico.Get("the wrong answer");
            Assert.AreEqual(0, fortythree);
            Assert.AreEqual(1, dico.Count);
            Assert.AreEqual(1, dico.NbUpdate);
            Assert.AreEqual(1, dico.NbHit);
            Assert.AreEqual(2, dico.NbMiss);
            Assert.AreEqual(0, dico.NbEviction);

            dico.Set("the answer", 24);
            Assert.AreEqual(24, dico.RawGet("the answer"));
            Assert.AreEqual(1, dico.Count);
            Assert.AreEqual(2, dico.NbUpdate);
            Assert.AreEqual(2, dico.NbHit);
            Assert.AreEqual(2, dico.NbMiss);
            Assert.AreEqual(0, dico.NbEviction);
        }

        [Test]
        public void Remove()
        {
            var dico = new LRUCache<string, int>();

            dico.Set("the answer", 42);
            Assert.AreEqual(42, dico.RawGet("the answer"));
            Assert.AreEqual(1, dico.Count);
            Assert.AreEqual(1, dico.NbUpdate);

            dico.Remove("the answer");
            Assert.AreEqual(0, dico.RawGet("the answer"));
            Assert.AreEqual(0, dico.Count);
            Assert.AreEqual(1, dico.NbUpdate);
        }

        [Test]
        public void Clear()
        {
            var dico = new LRUCache<string, int>();

            dico.Set("one", 1);
            dico.Set("two", 2);
            dico.Set("three", 3);
            Assert.AreEqual(1, dico.RawGet("one"));
            Assert.AreEqual(2, dico.RawGet("two"));
            Assert.AreEqual(3, dico.RawGet("three"));
            Assert.AreEqual(3, dico.Count);
            Assert.AreEqual(3, dico.NbUpdate);

            dico.Clear();
            Assert.AreEqual(0, dico.RawGet("one"));
            Assert.AreEqual(0, dico.RawGet("two"));
            Assert.AreEqual(0, dico.RawGet("three"));
            Assert.AreEqual(0, dico.Count);
        }

        [Test]
        public void TestLru()
        {
            var dico = new LRUCache<string, int>(2);

            dico.Set("one", 1);
            Assert.AreEqual(1, dico.RawGet("one"));
            Assert.AreEqual(1, dico.NbUpdate);
            Assert.AreEqual(0, dico.NbEviction);

            dico.Set("two", 2);
            Assert.AreEqual(1, dico.RawGet("one"));
            Assert.AreEqual(2, dico.RawGet("two"));
            Assert.AreEqual(2, dico.NbUpdate);
            Assert.AreEqual(0, dico.NbEviction);

            var u = dico.NbUpdate;
            var e = dico.NbEviction;
            for (int i = 0; i < 1000; ++i)
            {
                dico.Set("three", 3);
                Assert.AreEqual(0, dico.RawGet("one"));
                Assert.AreEqual(2, dico.RawGet("two"));
                Assert.AreEqual(3, dico.RawGet("three"));
                Assert.AreEqual(++u, dico.NbUpdate);
                Assert.AreEqual(++e, dico.NbEviction);

                dico.Set("one", 1);
                Assert.AreEqual(1, dico.RawGet("one"));
                Assert.AreEqual(0, dico.RawGet("two"));
                Assert.AreEqual(3, dico.RawGet("three"));
                Assert.AreEqual(++u, dico.NbUpdate);
                Assert.AreEqual(++e, dico.NbEviction);

                dico.Set("two", 2);
                Assert.AreEqual(1, dico.RawGet("one"));
                Assert.AreEqual(2, dico.RawGet("two"));
                Assert.AreEqual(0, dico.RawGet("three"));
                Assert.AreEqual(++u, dico.NbUpdate);
                Assert.AreEqual(++e, dico.NbEviction);
            }
        }

        private void Work(LRUCache<int, int> dico, int count)
        {
            var rand = new Random();
            for (int i = 0; i < 1000000; ++i)
            {
                int p = rand.Next(count);
                if (i % 4 == 0) dico.Set(p, p); else dico.Get(p);
            }
        }

        [Test]
        public void TestNoEviction()
        {
            const int size = 32;
            var dico = new LRUCache<int, int>(size);

            var tasks = new Task[Environment.ProcessorCount];
            for (int i = 0; i < tasks.Length; ++i) tasks[i] = Task.Factory.StartNew(() => Work(dico, size / 2));
            Task.WaitAll(tasks);
            Assert.AreEqual(0, dico.NbEviction);
            Assert.AreEqual(size / 2, dico.Count);
        }

        [Test]
        public void TestEvictions()
        {
            const int size = 32;
            var dico = new LRUCache<int, int>(size);

            var tasks = new Task[Environment.ProcessorCount];
            for (int i = 0; i < tasks.Length; ++i) tasks[i] = Task.Factory.StartNew(() => Work(dico, 1000 * size));
            Task.WaitAll(tasks);
            Assert.AreNotEqual(0, dico.NbEviction);
            Assert.AreEqual(dico.Capacity, dico.Count);
        }
    }
}
