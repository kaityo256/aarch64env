const int N = 10000;
double a[N], b[N], c[N], d[N];

void func() {
  for (int i = 0; i < N; i++) {
    d[i] = a[i] * b[i] + c[i];
  }
}
