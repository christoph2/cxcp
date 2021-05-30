#handle SIGRT_2 nostop noprint
set auto-load python-scripts on
display Xcp_State
handle SIG34 nostop noprint
break main
run

