find . -name nohup.out | while read file; do cat $file | grep Optimal | tail -n 1; done
