set remotetimeout 100
target extended-remote :4444
set remote hardware-watchpoint-limit 2
mon reset halt
flushregs
thb app_main
c