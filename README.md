# Aarch64 開発環境

## 概要

Aarch64上でのコードを開発するためのDocker環境。

## Dockerイメージの作成

まずリポジトリをcloneしよう。

```sh
git clone https://github.com/kaityo256/aarch64env.git
```

以下、研究室サーバ上で作業する場合には、`newgrp docker`を実行してDockerグループに所属しておく必要がある。

リポジトリの`docker`ディレクトリにDockerファイルがあるので、ビルドするとイメージができる。

```sh
cd aarch64env
cd docker
docker build -t kaityo256/aarch64env .
```

`kaityo256`の部分は自分の名前に変更すること。

なお、このイメージのビルドにはかなり時間がかかる。そこで、ビルド時間短縮のため、内部で並列ビルドを指定している箇所がある。`Dockerfile`の以下の部分だ。

```Dockerfile
RUN cd ${HOME} \
 && cd build/riken_simulator \
 && sed -i "369,372s:^:#:" SConstruct \
 && scons build/ARM/gem5.opt -j 20
```

最後の`-j 20`は、「20プロセスで並列ビルドせよ」という指示だ。研究室クラスタが20コアなのでこのような指定をしているが、家でビルドする場合には、環境に合わせて`-j 4`などとすると良い。

## コンテナの作成と接続

コンテナを立ち上げ、`user`というユーザ名でログインする。

```sh
docker run -it -u user kaityo256/aarch64env
```

すると、カレントディレクトリが`/`になっているので、`cd`で自分のホームに移動してから、サンプルディレクトリに入る。

```sh
cd
cd aarch64env/
cd samples
```

## 実行の確認

まずは`hello`に入ろう。

```sh
cd hello
```

そこには`hello.cpp`がある。単に`Hello Aarch64`と表示するだけのプログラムだ。

```cpp
#include <cstdio>

int main() {
  printf("Hello Aarch64\n");
}
```

まずは、普通にコンパイルしてみよう。`g++`でコンパイルすると、x86-64用の実行バイナリができる。

```sh
g++ hello.cpp
```

`a.out`というファイルができるので、それを実行してみよう。

```sh
$ ./a.out
Hello Aarch64
```

この実行バイナリがどういうファイルか、`file`コマンドで確認しよう。

```sh
$ file a.out
a.out: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, BuildID[sha1]=fa00082fcdd1581c6bfdab3aa8c2e45bc490e004, not stripped
```

`a.out`は、x86-64アーキテクチャ向けの64-bit ELFファイルであることがわかる。ELFとは「Executable and Linkable Format」の略だ。実行しているCPUはx86-64アーキテクチャなので、そのまま実行できる。

次に、Aarch64向けの実行バイナリを作成しよう。x86-64アーキテクチャ上で、全く別のアーキテクチャであるAarch64向けのバイナリを作るためには、クロスコンパイラを利用する。

```sh
aarch64-linux-gnu-g++-8 -static hello.cpp
```

`-static`オプションを忘れないこと。これは、「必要なライブラリを全て実行可能ファイルの中に含めよ」という意味だ。これを指定しないとShared Objectと呼ばれる共有ライブラリを利用する形となるが、デフォルトではx86-64の共有ライブラリを探しにいってしまい、実行に失敗する。

さて、できた`a.out`を実行してみよう。

```sh
$ ./a.out
bash: ./a.out: cannot execute binary file: Exec format error
```

Aarch64向けの実行ファイルをx86-64上で動かしたのでエラーが出た。ファイルを調べてみよう。

```sh
$ file a.out
a.out: ELF 64-bit LSB executable, ARM aarch64, version 1 (GNU/Linux), statically linked, for GNU/Linux 3.7.0, BuildID[sha1]=9a30661f857bba637707b5735bec5e7226b8e666, not stripped
```

ARM aarch64向けの64-bit ELFファイルであることがわかる。また、リンク形式が`statically linked`、つまり静的リンクになっていることにも注目。先ほどは`dynamically linked`になっていた。

このファイルはARM aarch64向けの実行ファイルであり、そのままではx86-64では実行できない。これをx86-64上で実行するのがエミュレータやシミュレータである。

まず、QEMUというエミュレータで実行してみよう。

```sh
$ qemu-aarch64 ./a.out
Hello Aarch64
```

ちゃんと実行できた。

次に、Gem5というシミュレータで実行してみよう。

```sh
$ gem5 ./a.out
gem5 Simulator System.  http://gem5.org
gem5 is copyrighted software; use the --copyright option for details.

gem5 compiled Jun 14 2020 12:18:22
gem5 started Jun 14 2020 13:26:06
gem5 executing on 910d217d1a52, pid 26
command line: /home/user/build/riken_simulator/build/ARM/gem5.opt /home/user/build/riken_simulator/configs/example/se.py -c ./a.out

/home/user/build/riken_simulator/configs/common/CacheConfig.py:51: SyntaxWarning: import * only allowed at module level
  def config_cache(options, system):
/home/user/build/riken_simulator/configs/common/CacheConfig.py:51: SyntaxWarning: import * only allowed at module level
  def config_cache(options, system):
Global frequency set at 1000000000000 ticks per second
warn: Cache line size is neither 16, 32, 64 nor 128 bytes.
warn: Sockets disabled, not accepting gdb connections
**** REAL SIMULATION ****
info: Entering event queue @ 0.  Starting simulation...
warn: readlink() called on '/proc/self/exe' may yield unexpected results in various settings.
      Returning '/home/user/aarch64env/samples/hello/a.out'
Hello Aarch64
Exiting @ tick 3900500 because exiting with last active thread context
```

いろいろ出力されるが、とりあえず最後の二行だけに注目すればよい。最後に`Hello Aarch64`と表示されていること、そして、tickとして3900500だけ実行されていることがわかる。tickが実行時間のようなもので、これが小さいほど実行時間が短いことを意味する。

QEMUもGem5も、実機で実行した場合と同じ結果が出力されるように動作を模倣するが、QEMUが結果のみ模倣しているのに対して、Gem5はキャッシュなどプロセッサ内部の実行状態もシミュレートしている。したがって、Gem5で実行する方が遅くなるが、「実機でどのくらいの時間で実行できるのか」はGem5で実行しないと予想できない。

なお、gem5は、実際には

```sh
~/build/riken_simulator/build/ARM/gem5.opt /home/user/build/riken_simulator/configs/example/se.py -c ./a.out
```

などとして実行するが、面倒なので、以下のようなエイリアスを定義している。

```sh
alias gem5='~/build/riken_simulator/build/ARM/gem5.opt /home/user/build/riken_simulator/configs/example/se.py -c'
```

これにより、

```sh
gem5 ./a.out
```

とすると、

```sh
~/build/riken_simulator/build/ARM/gem5.opt ~/build/riken_simulator/configs/example/se.py -c ./a.out
```

を実行したのと同じ結果になる。

## アセンブリの確認

次に`samples/multiplyadd`に移動しよう。いま`hello`にいるなら、一度上に移動してから`multiplyadd`に行けばよい。

```sh
cd ..
cd multiplyadd
```

ここには、`multiplyadd.cpp`がある。

```cpp
const int N = 10000;
double a[N], b[N], c[N], d[N];

void func() {
  for (int i = 0; i < N; i++) {
    d[i] = a[i] * b[i] + c[i];
  }
}
```

まずはx86向けにコンパイルしてみよう。

```sh
g++ -march=native -O3 -S multiplyadd.cpp
```

できた`multiplyadd.s`が作成されるので、見てみよう。

```asm
_Z4funcv:
.LFB0:
        .cfi_startproc
        leaq    d(%rip), %rdi
        leaq    b(%rip), %rsi
        leaq    c(%rip), %rcx
        leaq    a(%rip), %rdx
        xorl    %eax, %eax
        .p2align 4,,10
        .p2align 3
.L2:
        vmovapd (%rsi,%rax), %zmm0
        vmovapd (%rcx,%rax), %zmm1
        vfmadd132pd     (%rdx,%rax), %zmm1, %zmm0
        vmovapd %zmm0, (%rdi,%rax)
        addq    $64, %rax
        cmpq    $80000, %rax
        jne     .L2
        vzeroupper
        ret
```

ごちゃごちゃ書いてあるが、とりあえず`vfmadd132pd`という命令があることがわかるだろう。これが積和命令(multiply add)である。

次に、Aarch64向けにコンパイルしてみよう。

```sh
aarch64-linux-gnu-g++-8 -march=armv8-a+sve -O3 -S multiplyadd.cpp
```

この長ったらしいコンパイルコマンドを入力するのは面倒なので、`ag++`という名前でエイリアスを作ってある。以下を実行することでも同じ結果になる。

```sh
ag++ -O3 -S multiplyadd.cpp
```

アセンブリを見てみよう。

```asm
_Z4funcv:
.LFB0:
        .cfi_startproc
        mov     x1, 10000
        adrp    x6, a
        adrp    x5, b
        adrp    x4, c
        adrp    x2, d
        mov     x3, x1
        add     x6, x6, :lo12:a
        add     x5, x5, :lo12:b
        add     x4, x4, :lo12:c
        add     x2, x2, :lo12:d
        mov     x0, 0
        whilelo p0.d, xzr, x1
        ptrue   p1.d, all
        .p2align 3
.L2:
        ld1d    z1.d, p0/z, [x6, x0, lsl 3]
        ld1d    z2.d, p0/z, [x5, x0, lsl 3]
        ld1d    z0.d, p0/z, [x4, x0, lsl 3]
        fmla    z0.d, p1/m, z2.d, z1.d
        st1d    z0.d, p0, [x2, x0, lsl 3]
        incd    x0
        whilelo p0.d, x0, x3
        bne     .L2
        ret
```

やはりごちゃごちゃあるが、`fmla`という命令があることがわかる。これがARM aarch64における積和命令である。

## シミュレーション

`magnetic`というディレクトリに、「一週間でなれる！スパコンプログラマ」のDay 7で紹介された、磁場中の荷電粒子の運動のシミュレーションコードがある。

```sh
$ cd ..
$ cd magnetic
```

まずはx86向けにコンパイル、実行してみよう。

```sh
$ g++ -O3 mag.cpp
$ ./a.out
-0.447835 0.514910 -0.718851
```

1000粒子を100ステップ計算し、最後に0番目の粒子の座標を表示している。

同様にARM aarch64向けにクロスコンパイルし、gem5上で実行してみよう。

```sh
$ ag++ -O3 mag.cpp
$ gem5 ./a.out
(snip)
-0.447835 0.514910 -0.718851
Exiting @ tick 1425974000 because exiting with last active thread context
```

最後に、x86と同じ実行結果が表示され、かつ実行に`1425974000` tickかかったことがわかる。チューニングとは、最終的にかしこいアセンブリを出力し、このtick数を減らすのが目的となる。

## ライセンス

MIT
