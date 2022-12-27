#! /bin/csh -f
# HISTORY
# $Log:	test_out.csh,v $
# Revision 2.2  89/08/29  23:25:04  mrt
# 	Entered in to test suite
# 
# 

echo "Only thread_body(n) and That's all folks! should appear..."

./multi_out | grep -v '^Thread [0-5] out$'
