int fn(){

	write("call when previous is false\n");
	return 0;
}


int MAIN(){

	int a;

	for(a=1; fn()||a>0 ; a=a-1){
		write(a);
	}


	return 0;
}