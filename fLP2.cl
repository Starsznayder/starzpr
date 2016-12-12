#define WSPF 0.95
__kernel void fLP2(__global float2* x, __global float2* y, __global float2* sum1, int step, int PHI, int n)
{
int gl = get_global_size(0);
int tx = get_global_id(0);
__private float2 rez=0;
int g=tx;
int limit=0;
tx *=step;
if (step>100) limit=step-100;
while(tx+PHI+step<=n){
		for(int i=limit;i<step;i++){
			rez.x  = (WSPF* rez.x)+(y[tx+i+PHI].x * x[tx+i].x + y[tx+i+PHI].y * x[tx+i].y);
			rez.y  = (WSPF* rez.y)+(y[tx+i+PHI].y * x[tx+i].x - y[tx+i+PHI].x * x[tx+i].y);
		}
	sum1[g]=rez;
	rez=0;
	g+=gl;
	tx=g*step;
}
while (tx+PHI+step>n && tx+step<=n) {
		for(int i=limit;i<step;i++){
			if(tx+i+PHI<n){
			rez.x  = (WSPF* rez.x)+(y[tx+i+PHI].x * x[tx+i].x + y[tx+i+PHI].y * x[tx+i].y);
			rez.y  = (WSPF* rez.y)+(y[tx+i+PHI].y * x[tx+i].x - y[tx+i+PHI].x * x[tx+i].y);

			}
			else{
				rez.x  = (WSPF* rez.x)+(y[tx+i+PHI-n].x * x[tx+i].x + y[tx+i+PHI-n].y * x[tx+i].y);
				rez.y  = (WSPF* rez.y)+(y[tx+i+PHI-n].y * x[tx+i].x - y[tx+i+PHI-n].x * x[tx+i].y);
			}
		}
	sum1[g]=rez;
	rez=0;
	g+=gl;
	tx=g*step;
	}
}