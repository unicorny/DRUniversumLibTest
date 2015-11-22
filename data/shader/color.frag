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


float f_lacunarity = 2.0;
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


float sridged(vec3 p, float frequency, int octaveCount)
{
	float signal = 0.0;
	float value = 0.0;
	float weight = 1.0;

	float spectralWeightFrequenz = 1.0;

    // These parameters should be user-defined; they may be exposed in a
    // future version of libnoise.
	float offset = 1.0;
	float gain = 2.0;

	p *= frequency;

	for(int curOctave = 0; curOctave < octaveCount; curOctave++)
	{
		signal = snoise(p);

		// Make the ridges.
		signal = abs(signal);
		signal = offset - signal;
		
		// Square the signal to increase the sharpness of the ridges.
		signal *= signal;	

		// The weighting from the previous octave is applied to the signal.
	    // Larger values have higher weights, producing sharp points along the
	    // ridges.
	    signal *= weight;

		// Weight successive contributions by the previous signal.
    	weight = signal * gain;
    	if (weight > 1.0) weight = 1.0;
    	if (weight < 0.0) weight = 0.0;
   
    	// Add the signal to the output value.
    	value += (signal * pow(spectralWeightFrequenz, -1.0));

    	// Go to the next octave.
    	p *= f_lacunarity;
		spectralWeightFrequenz *= f_lacunarity;
	} 
	return value*1.2-1.0;
}


float sbillow(vec3 p, float frequency, int octaveCount)
{
	float value = 0.0;
	float signal = 0.0;
	float curPersistence = 1.0;

	p *= frequency;
//	cnoise(p*frequenzy);
	for(int curOctave = 0; curOctave < octaveCount; curOctave++)
	{
		signal = snoise(p);
		signal = 2.0*abs(signal)-1.0;
		value += signal * curPersistence;
		p *= f_lacunarity;
		curPersistence *= f_persistence;
	}
	value += 0.5;
	return value;
}

float select(float low, float high, float select, vec2 bounds, float edgeFallOff)
{
	//return select;
	//float s = 0.5+0.5*select;
	float alpha  = SCurve3((select-bounds.x+edgeFallOff)/(2.0*edgeFallOff));
	float alpha2 = SCurve3((select-bounds.y+edgeFallOff)/(2.0*edgeFallOff));

	if(select < (bounds.x - edgeFallOff)) 		  return low;
	else if(select < (bounds.x + edgeFallOff)) 	  return LinearInterp(low, high, alpha );
	else if (select < (bounds.y - edgeFallOff)) return high;
	else if (select < (bounds.y + edgeFallOff)) return LinearInterp(high, low, alpha2);
	else return low;
}

float curve(float value, vec2 points[10], int pointsCount)
{
// Find the first element in the control point array that has an input value
  // larger than the output value from the source module.
  int indexPos;
  for (indexPos = 0; indexPos < pointsCount; indexPos++) 
    if (value < points[indexPos].x) break;

  // Find the four nearest control points so that we can perform cubic
  // interpolation.
  ivec4 index = ivec4(clampi(indexPos - 2, 0, pointsCount - 1),  // 0, x 
					  clampi(indexPos - 1, 0, pointsCount - 1),  // 1, y
					  clampi(indexPos    , 0, pointsCount - 1),  // 2, z
					  clampi(indexPos + 1, 0, pointsCount - 1)); // 3, w

  // If some control points are missing (which occurs if the value from the
  // source module is greater than the largest input value or less than the
  // smallest input value of the control point array), get the corresponding
  // output value of the nearest control point and exit now.
  if (index.y == index.z) {
    return points[index.y].y;
  }
  
  // Compute the alpha value used for cubic interpolation.
  vec2 inputv = vec2(points[index.y].x, points[index.z].x);
  float alpha = (value - inputv.x) / (inputv.y - inputv.x);

  // Now perform the cubic interpolation given the alpha value.
  return CubicInterp (vec4(points[index.x].y,
						   points[index.y].y,
						   points[index.z].y,
						   points[index.w].y), alpha);
}

float terrace(float value, float points[6], int pointsCount)
{
  // Find the first element in the control point array that has a value
  // larger than the output value from the source module.
  int indexPos;
  for (indexPos = 0; indexPos < pointsCount; indexPos++) {
    if (value < points[indexPos]) {
      break;
    }
  }

  // Find the two nearest control points so that we can map their values
  // onto a quadratic curve.
  ivec2 index = ivec2(clampi(indexPos - 1, 0, pointsCount - 1),
					  clampi(indexPos    , 0, pointsCount - 1));

  // If some control points are missing (which occurs if the output value from
  // the source module is greater than the largest value or less than the
  // smallest value of the control point array), get the value of the nearest
  // control point and exit now.
  if (index.x == index.y) 
    return points[index.y];

  // Compute the alpha value used for linear interpolation.
  vec2 valuev = vec2(points[index.x], points[index.y]);
  float alpha = (value - valuev.x) / (valuev.y - valuev.x);

  // Squaring the alpha produces the terrace effect.
  alpha *= alpha;

  // Now perform the linear interpolation given the alpha value.
  return LinearInterp (valuev.x, valuev.y, alpha);
}

varying vec3 color;
uniform float time;
out vec4 result;
void main()
{
	vec3 colorTemp = vec3(0.0);//sOctaveNoise(color, 5, 2)*0.2+sridged(color, 12, 4)*0.2);
	float brown = (sOctaveNoise(vec3(color.x+time, color.y-time, color.z+time*10)+10, 2, 3)+sridged(color-time, 14, 4))*0.125+0.125;
	colorTemp.r += sbillow(color, 20,4)*0.2+0.2 + brown*2;
	colorTemp.g += (sOctaveNoise(color, 100, 6) *0.3 + sridged(color, 10, 4)*0.4)*0.5+0.5+brown;
	colorTemp.b += (sOctaveNoise(color, 120, 3) * 0.3) *0.5 + 0.5+brown;
	colorTemp = vec3(brown*2.56, vec2(brown));
	
	result = vec4(colorTemp, 1.0);
	//result.r = 1.0;
}
