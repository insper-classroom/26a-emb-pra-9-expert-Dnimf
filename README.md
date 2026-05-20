| Métrica / Task	| mpu_task |	fusion_task	| uart_task	| pwm_task |
|:---| --- | --- | --- | ---: |
| WCET |	20.115  ms |	10.022 ms |	40.012ms |	
| Jitter | 10.14 µs | 59.385 ns |1.252 µs |
| Deadline Miss Rate	| 66.667 % | 0% |	100% ** |		
| Stack Usage | 0.9033203125 | 4.4677734375% | 0.830078125% | 0.78125% |

 pwm não etendia a especificações ent não dava

| Métrica / Task	| mpu_task |	fusion_task	| uart_task	| pwm_task |
|:---| --- | --- | --- | ---: |
| WCET |	20.3064  ms |	10.085080000000062 ms |	40.12940000000003	  ms |20.048519999999826	|
| Jitter | 0.000005583266427458211	 ms | 0.004894725190739074	 ms |	0.06345127579480826	 ms | 0.03228093089070373 |
| Deadline Miss Rate	| 0 % | 0% |	0% |		0% |
| Stack Usage | 0.9033203125 | 8.935546875% | 1.7578125% | 1.953125% |


** depende pq se contar todos vtaskdelay tem que ter muito mais mas se for for o q estao executando,
mpu_task



