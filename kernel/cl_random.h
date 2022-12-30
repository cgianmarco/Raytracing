constant const float DEN = 1.0 / 4294967296.0;


typedef struct Random{

	uint seed;


} Random;


float intrnd (int * seed) // 1<=seed<=m
{

#if LONG_MAX > (16807*2147483647)
    int const a    = 16807;      //ie 7**5
    int const m    = 2147483647; //ie 2**31-1
        *seed = ((long)((*seed) * a))%m;
        return (*seed) / (float) m;
#else
    double const a    = 16807;      //ie 7**5
    double const m    = 2147483647; //ie 2**31-1

        double temp = (*seed) * a;
        *seed = (int) (temp - m * floor ( temp / m ));
        return (*seed) / (float) m;
#endif

}

uint rand_xorshift(uint * rng_state)
{
    uint state = *rng_state;
    // Xorshift algorithm from George Marsaglia's paper
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    *rng_state = state;
    return state;
}


uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}



// float rand(uint * seed)
// {
//     float r= wang_hash(*seed) * (DEN);
    
//     return r;
// }


float randRange(uint * seed, float min, float max){
    float result = wang_hash(*seed) * DEN * (max - min) + min;
    *seed += 10;
    return result;
}



float3 randVec(uint * seed, float min, float max){

    float3 random = { randRange(seed, min, max), 
                      randRange(seed, min, max),
                      randRange(seed, min, max) };

    return random;


}

float3 random_in_unit_sphere(uint * seed) {
    while (true) {
        float3 p = randVec(seed, -1, 1);
        if (pow(length(p), 2) >= 1) continue;
        return p;
    }
}