# �������֯����ϵ�ṹʵϰ���� Lab3.1

ѧ�ţ�

������

����ʦ��


---

## PART 1������Cacheģ�⣨100�֣�

  ʹ�ø���������ģ������ܣ�ʹ�ø����Ĳ���trace����ɵ���cache��ģ�⡣Ҫ��

1. �ڲ�ͬ�� Cache Size�� 32KB ~ 32MB�� �������£� Miss Rate �� Block Size�仯�����ƣ��ռ����ݲ���������ͼ����˵���仯ԭ��������4����ͬ��size��С��Ӧ������ͼ����40�֣�



2. �ڸ��� Cache Size �������£���128KB��512KB�� Miss Rate �� Associativity�� 1-32�� �仯�����ƣ��ռ����ݲ���������ͼ����˵���仯ԭ��������2��4��8��16��32��Ӧ������ͼ����40�֣�



3. �Ƚ� Write Through �� Write Back�� Write Allocate �� No-write allocate ���ܷ�����ʱ�Ĳ��졣��20�֣�


## PART 2. ��lab2�еĴ���������ģ��������������ѡ��

��Lab 2�е���ˮ��ģ�������������в��Գ���
�ò�����cache���������£�

| Level | Capacity | Associativity | Line size(Bytes) | WriteUp Polity | Hit Latency  |
| ----- | -------- | ------------- | ---------------- | -------------- | ------------ |
| L1    | 32 KB    | 8 ways        | 64               | write Back     | 1 cpu cycle  |
| L2    | 256 KB   | 8 ways        | 64               | write Back     | 8 cpu cycle  |
| LLC   | 8 MB     | 8 ways        | 64               | write Back     | 20 cpu cycle |

1. ���Գ�����ѡ�����Խ����ȷ������ӡ��ִ̬�е�ָ������CPI����40�֣�

-  �ܹ�����ˮ��ģ����Эͬ��������10�֣�
-  �ܹ�ģ�����cache����10�֣�
-  ���Գ���������ȷ������ӡ����ִ̬��ָ�������Լ���Ӧ��CPI����4��*5��

2���Ա�lab2�е���ˮ��ģ����������CPI�ı仯ԭ�򡣣�10�֣�