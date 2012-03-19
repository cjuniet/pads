using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ProjectEuler
{
    public class Primes
    {
        public static ulong modexp(ulong a, ulong b, ulong n)
        {
            if (b == 0) return 1;
            if (b == 1) return a % n;
            if ((b & 0x1) == 1)
            {
                ulong x = a % n;
                return (x * modexp(a, b - 1, n)) % n;
            }
            else
            {
                ulong x = a % n;
                x = (x * x) % n;
                return (modexp(x, b / 2, n) % n);
            }
        }

        private static bool miller_rabin_witness(ulong n, ulong a)
        {
            ulong t = 0;
            ulong u = n - 1;
            while ((u & 0x1) == 0) { ++t; u >>= 1; }
            ulong x = modexp(a, u, n);
            for (ulong i = 1; i <= t; ++i)
            {
                ulong y = (x * x) % n;
                if (y == 1 && x != 1 && x != n - 1) return true;
                x = y;
            }
            if (x != 1) return true;
            return false;
        }

        public static bool fast_miller_rabin(ulong n)
        {
            if (n <= 2) return n == 2;
            if ((n & 0x1) == 0) return false;
            if (miller_rabin_witness(n, 2)) return false;
            if (miller_rabin_witness(n, 3)) return false;
            if (miller_rabin_witness(n, 5)) return false;
            if (miller_rabin_witness(n, 7)) return false;
            if (miller_rabin_witness(n, 11)) return false;
            if (miller_rabin_witness(n, 13)) return false;
            if (miller_rabin_witness(n, 17)) return false;
            if (miller_rabin_witness(n, 31)) return false;
            if (miller_rabin_witness(n, 73)) return false;
            if (miller_rabin_witness(n, 61)) return false;
            return true;
        }
    }
}