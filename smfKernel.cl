__kernel void smfKernel(__global float* sumR1,__global float* sumI1,__global float* sumR2, int crrLen)
//---------------------------------------Sumowanie hierarchiczne----------------------------------
//---------------------------------------------oblicza k i h--------------------------------------
{
int tx = get_global_id(0);
		if (tx<crrLen){
			float tR1=sumR1[tx];
			float tI1=sumI1[tx];
			float tR2=sumR2[tx];
			tR1+=sumR1[tx+crrLen];
			tI1+=sumI1[tx+crrLen];
			tR2+=sumR2[tx+crrLen];
			sumR1[tx]=tR1;
			sumI1[tx]=tI1;
			sumR2[tx]=tR2;
		}
}