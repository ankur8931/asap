int add_integers(int a, int b)
{
  return a+b;
}

double add_floats(double a, double b)
{
  return a+b;
}

void swap_ints(int *a, int *b)
{
  int c = *a;
  *a = *b;
  *b = c;
}

void sort(char *A)
{
  int i,flag = 1;
  char temp;
  /* bubble-sort */
  
  while (flag) {
    flag = 0;
    for (i=0; A[i+1]!='\0'; i++) 
      if (A[i] > A[i+1]) {
	temp = A[i];
	A[i] = A[i+1];
	A[i+1] = temp;
	flag = 1;
      }
  }
}
