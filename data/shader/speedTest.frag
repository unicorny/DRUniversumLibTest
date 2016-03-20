#version 400
// simple fragment shader

// 'time' contains seconds since the program was linked.
//uniform sampler2D texture;

//libnoise helper
float SCurve3 (float a) {return (a * a * (3.0 - 2.0 * a));}
float LinearInterp (float n0, float n1, float a) {return ((1.0 - a) * n0) + (a * n1);}
vec4 LinearInterpColor (vec4 color0, vec4 color1, float alpha)
{
	return (color1 * alpha) + (color0 * (1.0 - alpha));
}
int clampi(int x, int minVal, int maxVal) 
{
  	if(x < minVal) return minVal;
	else if (x > maxVal) return maxVal;
  	else return x;
}

float clampf(float x, float minVal, float maxVal) 
{
  	if(x < minVal) return minVal;
	else if (x > maxVal) return maxVal;
  	else return x;
}

float CubicInterp (vec4 n, float a)
{//      0 1 2 3
//vec4 = x y z w
  float p = (n.w - n.z) - (n.x - n.y);
  float q = (n.x - n.y) - p;
  float r = n.z - n.x;
  float s = n.y;
  return p * a * a * a + q * a * a + r * a + s;
}


/// Square root of 3.
const float SQRT_3 = 1.7320508075688772935;
int IntValueNoise3D (ivec3 p, int seed)
{
  // All constants are primes and must remain prime in order for this noise
  // function to work correctly.
  int n = (
      1619    * p.x
    + 31337    * p.y
    + 6971    * p.z
    + 1013 * seed)
    & 0x7fffffff;
  n = (n >> 13) ^ n;
  return (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
}

float ValueNoise3D (vec3 p, int seed)
{
  return 1.0 - int(IntValueNoise3D (ivec3(p), seed) / 1073741824.0);
}

float voronoi(vec3 p, int seed, float frequenzy, float displacement, bool enableDistance)
{
	// This method could be more efficient by caching the seed values.  Fix
  // later.

  p *= frequenzy;
  
  ivec3 Int = ivec3(p.x > 0.0? int(p.x): int(p.x) - 1,
					p.y > 0.0? int(p.y): int(p.y) - 1,
					p.z > 0.0? int(p.z): int(p.z) - 1);

  float minDist = 2147483647.0;
  vec3 candidate = vec3(0.0);

  // Inside each unit cube, there is a seed point at a random position.  Go
  // through each of the nearby cubes until we find a cube with a seed point
  // that is closest to the specified position.
  for (int zCur = Int.z - 2; zCur <= Int.z + 2; zCur++) {
    for (int yCur = Int.y - 2; yCur <= Int.y + 2; yCur++) {
      for (int xCur = Int.x - 2; xCur <= Int.x + 2; xCur++) {

        // Calculate the position and distance to the seed point inside of
        // this unit cube.
		vec3 Pos = vec3(xCur + ValueNoise3D (vec3(xCur, yCur, zCur), seed    ),	
						yCur + ValueNoise3D (vec3(xCur, yCur, zCur), seed + 1),
						zCur + ValueNoise3D (vec3(xCur, yCur, zCur), seed + 2));
		vec3 Dist = Pos-p;
        float dist = Dist.x * Dist.x + Dist.y * Dist.y + Dist.z * Dist.z;

        if (dist < minDist) {
          // This seed point is closer to any others found so far, so record
          // this seed point.
          minDist = dist;
		  candidate = Pos;
        }
      }
    }
  }

  float value;
  if (enableDistance)
  {
    // Determine the distance to the nearest seed point.
	vec3 Dist = candidate - p;
    value = (length(Dist)) * SQRT_3 - 1.0;
  } 
  else
  {
    value = 0.0;
  }

  // Return the calculated distance with the displacement value applied.
  return value + (displacement * ValueNoise3D (vec3(floor (candidate)), seed));
}


// noise helper
vec4 permute(vec4 x) {return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r) {return 1.79284291400159 - 0.85373472095314 * r;}
vec3 fade(vec3 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float snoise(vec3 v)
{ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}


float f_lacunarity = 2.2;
float f_persistence = 0.5;
float sOctaveNoise(vec3 p, float frequenzy, int octaveCount)
{
	float value = 0.0;
	float curPersistence = 1.0;
//	cnoise(p*frequenzy);
	for(int curOctave = 0; curOctave < octaveCount; curOctave++)
	{
		value += snoise(p*frequenzy) * curPersistence;
		p *= f_lacunarity;
		curPersistence *= f_persistence;
	}
	return value;
}


varying vec3 color;
out vec4 result;
void main()
{
	//float v = sOctaveNoise(color, 10.0, 10)*0.5+0.5;
	float r = sOctaveNoise(color, 10.0, 10)*0.5+0.5;
	float g = sOctaveNoise(color+10, 20.0, 5)*0.5+0.5;
	float b = sOctaveNoise(color*2, 10.0, 10)*0.5+0.5;
	
	result = vec4(r, g, b, 1.0);
	//result.r = 1.0;
}
