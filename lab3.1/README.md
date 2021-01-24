# 计算机组织与体系结构实习报告 Lab3.1

学号：

姓名：

大班教师：


---

## PART 1、单层Cache模拟（100分）

  使用附件中所给模拟器框架，使用给定的测试trace，完成单层cache的模拟。要求：

1. 在不同的 Cache Size（ 32KB ~ 32MB） 的条件下， Miss Rate 随 Block Size变化的趋势，收集数据并绘制折线图。并说明变化原因。至少有4个不同的size大小对应的折线图。（40分）



2. 在给定 Cache Size 的条件下，如128KB和512KB， Miss Rate 随 Associativity（ 1-32） 变化的趋势，收集数据并绘制折线图。并说明变化原因。至少有2、4、8、16、32对应的折线图。（40分）



3. 比较 Write Through 和 Write Back、 Write Allocate 和 No-write allocate 的总访问延时的差异。（20分）


## PART 2. 与lab2中的处理器性能模拟器联调。（可选）

与Lab 2中的流水线模拟器联调，运行测试程序。
该测试中cache的配置如下：

| Level | Capacity | Associativity | Line size(Bytes) | WriteUp Polity | Hit Latency  |
| ----- | -------- | ------------- | ---------------- | -------------- | ------------ |
| L1    | 32 KB    | 8 ways        | 64               | write Back     | 1 cpu cycle  |
| L2    | 256 KB   | 8 ways        | 64               | write Back     | 8 cpu cycle  |
| LLC   | 8 MB     | 8 ways        | 64               | write Back     | 20 cpu cycle |

1. 测试程序自选，测试结果正确，并打印动态执行的指令数和CPI。（40分）

-  能够与流水线模拟器协同工作。（10分）
-  能够模拟多层次cache。（10分）
-  测试程序运行正确，并打印出动态执行指令数，以及相应的CPI。（4分*5）

2、对比lab2中的流水线模拟器，分析CPI的变化原因。（10分）