all:
		g++ -o hw2.so -shared -fPIC hw2.cpp -ldl 
run:
		LD_PRELOAD=./hw2.so ./0417
clean:
		rm -rf 0417 hw2.so monitor.out
