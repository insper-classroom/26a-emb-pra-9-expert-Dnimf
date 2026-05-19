| Métrica / Task	| mpu_task |	fusion_task	| uart_task	| pwm_task |
|:---| --- | --- | --- | ---: |
| WCET |	20.06548 ms |	10.00804 ms |	40.00532 ms |	
| Jitter | 0,0282 ms | 0,004 ms |	0,00068 ms |
| Deadline Miss Rate	| 100 % | 100%* |	100% ** |		
| Stack Usage |

*ou 0% porque todas as ultrapasagens foram menores doque 0.01ms 
** depende pq se contar todos vtaskdelay tem que ter muito mais mas se for for o q estao executando,
mpu_task

---------------------
Δ20.06548
Δ19.9808 
Δ19.97884 
Δ19.98144 
Δ19.98072 
Δ19.98184 

Δ29,98628 
30,01448
Δ60,00184 ms
Δ59,99784 

59,99932 

_________
Δ40.00416 
Δ40.00532 
Δ40.0008 
Δ40.00188 
Δ40.00316 
Δ40.00204 



--------------------
uart 
Δ40.00352
Δ40.00232 
Δ40.00316 
Δ40.002 
Δ40.0024 
Δ40.0042 
