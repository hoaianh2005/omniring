#include <iostream>
using namespace std;

// m(x) = x^10 + x^3 + 1
const int MOD = 0b10000001001;

int degree(int x){
    int deg = -1;
    while(x){
        x >>= 1;
        deg++;
    }
    return deg;
}
int poly_mul(int a, int b){
    int result = 0;
    cout << "Multiplying: " << a << " * " << b << endl;
    while(b){
        if(b & 1){
            cout << "XOR with " << a << endl;
            result ^= a;
        }
        a <<= 1;
        b >>= 1;
    }
    cout << "Mul result = " << result << endl;
    return result;
}
int poly_mod(int a){
    while(degree(a) >= degree(MOD)){
        int shift = degree(a) - degree(MOD);
        cout << "Reduce: " << a << " XOR " << (MOD << shift) << endl;
        a ^= MOD << shift;
    }
    return a;
}
int poly_div(int a, int b){
    int q = 0;
    while(degree(a) >= degree(b)){
        int shift = degree(a) - degree(b);
        q ^= (1 << shift);
        a ^= b << shift;
    }
    return q;
}
int inverse(int a){
    int r0 = MOD;
    int r1 = a;
    int t0 = 0;
    int t1 = 1;
    int step = 0;
    while(r1 != 1){
        cout << "\nStep " << step << endl;
        int q = poly_div(r0, r1);
        cout << "q = " << q << endl;
        int r2 = r0 ^ poly_mul(q, r1);
        r2 = poly_mod(r2);
        int t2 = t0 ^ poly_mul(q, t1);
        t2 = poly_mod(t2);
        cout << "r2 = " << r2 << endl;
        cout << "t2 = " << t2 << endl;
        r0 = r1;
        r1 = r2;
        t0 = t1;
        t1 = t2;
        step++;
    }

    return poly_mod(t1);
}
int main(){
    int a = 523;
    int b = 1015;
    cout << "Finding inverse of a = " << a << endl;
    int ainv = inverse(a);
    cout << "\na^-1 = " << ainv << endl;
    cout << "\n---------------------------\n";
    cout << "Finding inverse of b = " << b << endl;
    int binv = inverse(b);
    cout << "\nb^-1 = " << binv << endl;
    return 0;
}