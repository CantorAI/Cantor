#How to host docs on 
https://docs.readthedocs.io/en/stable/tutorial/index.html

#Debug on Linux set ptrace_scope to 0
echo "0"|sudo tee /proc/sys/kernel/yama/ptrace_scope
