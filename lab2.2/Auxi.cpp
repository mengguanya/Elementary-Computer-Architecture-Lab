unsigned long long ext_signed(unsigned long long val, int s, int tar) {
    if (s < 1)
        return val;
    int t_l = s;
    unsigned long long t = val;
    t >>= (t_l - 1);
    unsigned long long Bit = 1;
    if (t == 0)
        return val;
    else {
        while (t_l) {
            Bit *= 2;
            t_l--;
        }
        for (int i = s; i < tar; i++) {
            val += Bit;
            Bit *= 2;
        }
    }
    return val;
}

unsigned long long getbit(long long x, int s, int e) {
    int t_s = s,
        t_e = e;
    unsigned long long T = 1;
    unsigned long long Bit = 1;
    while (t_s > 0) {
        T *= 2;
        Bit *= 2;
        t_s--;
    }
    while (t_e - s > 0) {
        Bit *= 2;
        T += Bit;
        t_e--;
    }
    x = x & T;
    for (int i = 0; i < s; i++) {
        x /= 2;
    }
    return x;
}