Switch ledSwitch {mqtt=">[broker:/saloon/led:command:ON:HIGH],>[broker:/saloon/led:command:OFF:LOW]"}

Group AllTemp
Group AllHumi 

Number Temp "Temperature [%.1f]" (AllTemp) {mqtt="<[broker:saloon/temperature:state:default]"} 
Number Humi "Humidity [%.1f]" (AllHumi) {mqtt="<[broker:saloon/humidity:state:default]"} 


Number Temp2 "Temperature2 [%.1f]" (AllTemp) {mqtt="<[broker:alex/temperature:state:default]"} 
Number Humi2 "Humidity2 [%.1f]" (AllHumi) {mqtt="<[broker:alex/humidity:state:default]"} 


