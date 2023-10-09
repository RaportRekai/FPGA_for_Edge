#include<iostream>
#include<ap_int.h>

using namespace std;

// Defining Custom Datatype
typedef ap_uint<9> bbxy;
typedef ap_uint<10> accuracy;
typedef ap_uint<1> int1;
typedef ap_uint<3> int3;

bbxy nbbx1 = 0, nbby1 = 0, nbbx2 = 0, nbby2= 0;
accuracy nscore=0;
int3 nleaves=0;
int1 nresult=0, ncompute=0;

void regressor(int1 start, int1 &compute, int3 &leaves, accuracy &scores_out, bbxy &out_bbx1, bbxy &out_bby1, bbxy &out_bbx2, bbxy &out_bby2);

int main(){
	regressor(1, ncompute, nleaves, nscore, nbbx1, nbby1, nbbx2, nbby2);
}
