int a[5][10];

int MAIN()
{
	int i,j;
	int k,l;
	for(i=0; i<5; i=i+1)
		for(j=0; j<10; j=j+1)
			a[i][j] = i-j;
	write("hello\n");
	for(i=0; i<5; i=i+1)
			a[i][0] = i;

	write("xxx\n");
  return 0;
  
}

