<!-- omit in toc -->
# Pipeline

`Pipeline`はマルチスレッドによる並列パイプラインシステムを容易に実現するためのC++向け Header-only Library です。

- `c++20`以降
- `boost/container`

を前提としています。

- [開発動機](#開発動機)
- [仕様](#仕様)
- [TIPS](#tips)
  - [実行ラインのコールバック](#実行ラインのコールバック)
  - [サイクルごとの手動実行](#サイクルごとの手動実行)

## 開発動機

一般に、並列パイプラインは以下の三つで表現できます。

- `ExecutionLine`（実行ライン）
- `ExecutionUnit`（実行ユニット）
- `Gate`（ゲート）

この事実に従えば、例えば更新と描画が直列している状態からそれぞれを分離して並列パイプライン化することは本来簡単な筈です。少なくとも概念上は実行ラインを一つ分岐させ、同期用のゲートを用意すれば事足りる筈です。また、トリプルバッファリングや、あるいはもっと複雑な構造であれ、同じように書ける筈ですし、そうあるべきです。

```text
Line1: ---A---B-|--C--|--D---E---F---G---|---
Line2: ---X-----|--Y--⤵-----------|---Z--|---
Line3: ---α-----------|---β---γ---|---δ------

※ | はゲートを示すものとする。
※ この図においてゲートは4つ存在する。
※ 上下が揃っているゲートは、それぞれのラインに結びついていることを示す。例えば左から一番目のゲートはLine1, Line2に結びついている。
※ ⤵はLineを跨いでいることを示す。すなわち左から二番目のゲートはLine1, Line3に結びついている。
```

しかしながら実際にそれらをコードに落とし込もうとすると、随分面倒で、まったく直感的ではありません。何故なら適切な抽象化が欠けているからです。適切な抽象化さえあれば、本来は上のような図をイメージして、それを直接的に書ける筈です。

**この観点の下では実行ラインに実行ユニットを配置し、同期用のゲートを配置することでパイプライン全体を構成していくことになるでしょう。**

```c++
class SampleUnit final : public Pipeline::ExecutionUnit {
public:
    void run(void* ctx) override {
    }
};

/* イグニッションゲート。後述参照 */
Pipeline::Gate ignitionGate{};

Pipeline::ExecutionLine line1{ignitionGate};
Pipeline::ExecutionLine line2{ignitionGate};
Pipeline::ExecutionLine line3{ignitionGate};

Pipeline::Gate gateA, gateB, gateC, gateD;

SampleUnit unitA, unitB, unitC, unitD, unitE, unitF, unitG;
SampleUnit unitX, unitY, unitZ;
SampleUnit unitAlpha, unitBeta, unitGamma, unitDelta;

line1.addExecutionUnit(unitA);
line1.addExecutionUnit(unitB);
line1.addGate(gateA);
line1.addExecutionUnit(unitC);
line1.addGate(gateB);
line1.addExecutionUnit(unitD);
line1.addExecutionUnit(unitE);
line1.addExecutionUnit(unitF);
line1.addExecutionUnit(unitG);
line1.addGate(gateE);

line2.addExecutionUnit(unitX);
line2.addGate(gateA);
line2.addExecutionUnit(unitY);
line2.addGate(gateC);
line2.addExecutionUnit(unitZ);
line2.addGate(gateE);

line3.addExecutionUnit(unitAlpha);
line3.addGate(gateB);
line3.addExecutionUnit(unitBeta);
line3.addExecutionUnit(unitGamma);
line3.addGate(gateC);
line3.addExecutionUnit(unitDelta);
```

一度パイプラインを構成したならば、あとは開始するだけです。ここで **`IgnitionGate`（イグニッションゲート）という概念を導入します。これはいわばパイプライン全体を駆動させるためのスイッチです。**

```c++
/* 登録された情報からそれらを参照する実行ラインを構築 */
line1.buildOn(ctx);
line2.buildOn(ctx);
line3.buildOn(ctx);

/* イグニッションゲートを開放（イグニッション = 点火） */
ignitionGate.unlock();
```

**一度をパイプライン全体を定義しイグニッション（点火）したならば、あとは全自動でパイプラインが駆動し続けます。** 個々の実行ラインはゲートが解放されるのを待ち、ゲートが解放されたら次の実行ユニットを呼び出し、ゲートがあれば開放されるのを待ち、また次の実行ユニットがあればそれを呼び出し……という風に動作します。

上に記したコード断片はあくまでも概念的なものですが、これを見れば回路による抽象化が並列パイプラインの構築においてどれだけ役立つかが分かるかと思います。頭の中にあるパイプラインの構成図をそのままコードに対応させ、イグニッションゲートを開放すればあとは自動的に動いてくれるのです――全自動で同期しつつ！

これが`Pipeline`の提供する機能であり、また開発動機となります。

## 仕様

- 一つのゲートに接続可能な実行ラインの最大数は32
- 一つの実行ラインに配置できる要素（実行ユニットあるいはゲート）の最大数は64
- `ExcutionLine`および`Gate`は自身のコールバックに対して所有権を持たないため、寿命の管理はユーザーの責務

## TIPS

### 実行ラインのコールバック

実行ラインはスレッドで実装されています。ところでスレッドの開始時と終了時に適切な関数を呼びたいという動機があるでしょう。例えば`COM`を利用するなら`CoInitializeEx`, `CoUninitialize`を呼ぶ必要があるのは言うまでもありません。こうした場合をサポートするために、またエラーハンドリングのために、**`Pipeline::ExecutionLine`にはコールバックが定義されています。**

```c++
class ExecutionLine {
public:
    class Callback {
    public:
        virtual ~Callback() noexcept = default;
        virtual void onStart(void* ctx) = 0;
        virtual void onFinished(void* ctx) noexcept = 0;
        virtual void onError(void* ctx, std::exception_ptr error) noexcept = 0;
        virtual void onExecutionError(void* ctx, ExecutionUnit& executionUnit, std::exception_ptr error) noexcept = 0;
    };
};
```

`onStartup`は実行ラインの開始時に一度だけ呼ばれ`onFinished`はその逆、実行ラインの終了時に一度だけ呼ばれます。ここで、`onFinished`は`onStartup`で例外が発生しても呼ばれることは特筆に値します。というのは、そうしなければ`onStartup`の実装で強い例外保証を提供する必要があるためです。この保証を行うためにはエラー発生時の振る舞いを`onFinished`と殆ど同一にせざるを得ず、実装上あまりにも面倒であるため **`onFinished`は`onStartup`で例外が発生しても必ず呼ばれる**ようにしています。

**また、エラーが発生して実行ラインが異常終了した場合、パイプライン回路自体はブロッキングされたまま停止します。** 何故なら実行ラインが異常終了したにも関わらず次のサイクルが実行されるなどあってはならないことだからです。**したがって実行ラインの異常終了時の振る舞いとして最も適切なのは、その実行ラインが属するパイプライン回路全体を終了させることになるでしょう。** この処理がユーザーに委ねられているのは、回路全体を知る（把握している）のがユーザーでありライブラリではないためです。

### サイクルごとの手動実行

既に述べたように、`Pipeline`は一度イグニッションゲートを開放したら後は自律的に駆動するような作りになっています。では**ワンサイクルで実行を止め、何らかの入力が生じるまでサイクルの実行を停止させるようなことは出来るのでしょうか？**

**もちろん出来ます。** 対象のパイプラインに対し

- ワンサイクルを定義するためのゲート
- 入力に応じてブロッキング解除するだけの実行ユニット
- そのユニットを動かすための実行ライン

を用意すればいいのです。どういうことかというと、例えば下図のようなパイプラインがあったとします。

```text
Line1: ---A---B--|---C---D---
Line2: ---X------|---Y---Z---
```

これに対して

```text
Line1       : ---A---B--|---C---D---|---
Line2       : ---X------|---Y---Z---|---
BlockingLine: ---Blocker------------|---
```

とすればいいのです。Blockerは入力が生じるまで待機し入力が生じたら終了するだけの単純な実行ユニットです。したがって入力が生じるまでBlockerは終了せず、それゆえサイクルゲートの再開放が生じずワンサイクルで実行が停止したままになります。
