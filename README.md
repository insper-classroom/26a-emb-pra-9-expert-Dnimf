| Métrica / Task	| mpu_task |	fusion_task	| uart_task	| pwm_task |
|:---| --- | --- | --- | ---: |
| WCET |	20.06548 ms |	10.00804 ms |	40.00532 ms |	
| Jitter | 0,0282 ms | 0,004 ms |	0,00068 ms |
| Deadline Miss Rate	| 100 % | 0% |	100% ** |		
| Stack Usage | 0.9033203125 | 4.4677734375% | 0.830078125% | 0.78125% |
 pwm não etendia a especificações ent não dava

| Métrica / Task	| mpu_task |	fusion_task	| uart_task	| pwm_task |
|:---| --- | --- | --- | ---: |
| WCET |	20.02944  ms |	10.07032 ms |	40.04556  ms |	20.0254 |
| Jitter | 0,0282 ms | 0,004 ms |	0,00068 ms |
| Deadline Miss Rate	| 100 % | 0% |	100% ** |		
| Stack Usage | 0.9033203125 | 4.4677734375% | 0.830078125% | 0.78125% |
** depende pq se contar todos vtaskdelay tem que ter muito mais mas se for for o q estao executando,
mpu_task

Δ10.07032  Δ40.01668  Δ20.00164 
Δ10.036    Δ40.04408  Δ19.99208 
Δ10.01972  Δ40.04556  Δ20.02416 
Δ10.02724  Δ39.99864  Δ20.01452 
Δ10.03684  Δ40.04316  Δ20.0254 
