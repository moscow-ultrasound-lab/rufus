
#if 0

//http://www.easyrgb.com/math.php?MATH=M1#text1

//------------------------------
//
// XYZ->RGB
//
//------------------------------

var_X = X / 100        //Where X = 0 Ц  95.047
var_Y = Y / 100        //Where Y = 0 Ц 100.000
var_Z = Z / 100        //Where Z = 0 Ц 108.883

var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986
var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415
var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570

if ( var_R > 0.0031308 ) var_R = 1.055 * ( var_R ^ ( 1 / 2.4 ) ) - 0.055
else                     var_R = 12.92 * var_R
if ( var_G > 0.0031308 ) var_G = 1.055 * ( var_G ^ ( 1 / 2.4 ) ) - 0.055
else                     var_G = 12.92 * var_G
if ( var_B > 0.0031308 ) var_B = 1.055 * ( var_B ^ ( 1 / 2.4 ) ) - 0.055
else                     var_B = 12.92 * var_B

R = var_R * 255
G = var_G * 255
B = var_B * 255

//------------------------------
//
// RGB->XYZ
//
//------------------------------

var_R = ( R / 255 )        //Where R = 0 Ц 255
var_G = ( G / 255 )        //Where G = 0 Ц 255
var_B = ( B / 255 )        //Where B = 0 Ц 255

if ( var_R > 0.04045 ) var_R = ( ( var_R + 0.055 ) / 1.055 ) ^ 2.4
else                   var_R = var_R / 12.92
if ( var_G > 0.04045 ) var_G = ( ( var_G + 0.055 ) / 1.055 ) ^ 2.4
else                   var_G = var_G / 12.92
if ( var_B > 0.04045 ) var_B = ( ( var_B + 0.055 ) / 1.055 ) ^ 2.4
else                   var_B = var_B / 12.92

var_R = var_R * 100
var_G = var_G * 100
var_B = var_B * 100

//Observer. = 2Ў, Illuminant = D65
X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805
Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722
Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505

//------------------------------
//
// XYZ->Yxy
//
//------------------------------

//Where X = 0 Ц  95.047       Observer. = 2Ў, Illuminant = D65
//Where Y = 0 Ц 100.000
//Where Z = 0 Ц 108.883

Y = Y
x = X / ( X + Y + Z )
y = Y / ( X + Y + Z )

//------------------------------
//
// Yxy -> XYZ
//
//------------------------------

//Where Y = 0 Ц 100
//Where x = 0 Ц 1
//Where y = 0 Ц 1

X = x * ( Y / y )
Y = Y
Z = ( 1 - x - y ) * ( Y / y )

//------------------------------
//
// XYZ -> Hunter-Lab
//
//------------------------------

(H)L = 10 * sqrt( Y )
(H)a = 17.5 * ( ( ( 1.02 * X ) - Y ) / sqrt( Y ) )
(H)b = 7 * ( ( Y - ( 0.847 * Z ) ) / sqrt( Y ) )

//------------------------------
//
// Hunber-Lab -> XYZ
//
//------------------------------

var_Y = (H)L / 10
var_X = (H)a / 17.5 * (H)L / 10
var_Z = (H)b / 7 * (H)L / 10

Y = var_Y ^ 2
X = ( var_X + Y ) / 1.02
Z = -( var_Z - Y ) / 0.847

//------------------------------
//
// XYZ -> CIE-L*ab
//
//------------------------------
var_X = X / ref_X          //ref_X =  95.047  Observer= 2Ў, Illuminant= D65
var_Y = Y / ref_Y          //ref_Y = 100.000
var_Z = Z / ref_Z          //ref_Z = 108.883

if ( var_X > 0.008856 ) var_X = var_X ^ ( 1/3 )
else                    var_X = ( 7.787 * var_X ) + ( 16 / 116 )
if ( var_Y > 0.008856 ) var_Y = var_Y ^ ( 1/3 )
else                    var_Y = ( 7.787 * var_Y ) + ( 16 / 116 )
if ( var_Z > 0.008856 ) var_Z = var_Z ^ ( 1/3 )
else                    var_Z = ( 7.787 * var_Z ) + ( 16 / 116 )

CIE-L* = ( 116 * var_Y ) - 16
CIE-a* = 500 * ( var_X - var_Y )
CIE-b* = 200 * ( var_Y - var_Z )

//------------------------------
//
// CIE-L*ab -> XYZ
//
//------------------------------

var_Y = ( CIE-L* + 16 ) / 116
var_X = CIE-a* / 500 + var_Y
var_Z = var_Y - CIE-b* / 200

if ( var_Y^3 > 0.008856 ) var_Y = var_Y^3
else                      var_Y = ( var_Y - 16 / 116 ) / 7.787
if ( var_X^3 > 0.008856 ) var_X = var_X^3
else                      var_X = ( var_X - 16 / 116 ) / 7.787
if ( var_Z^3 > 0.008856 ) var_Z = var_Z^3
else                      var_Z = ( var_Z - 16 / 116 ) / 7.787

X = ref_X * var_X     //ref_X =  95.047  Observer= 2Ў, Illuminant= D65
Y = ref_Y * var_Y     //ref_Y = 100.000
Z = ref_Z * var_Z     //ref_Z = 108.883

//------------------------------
//
// CIE-L*ab -> CIE-L*CH
//
//------------------------------

var_H = arc_tangent( CIE-b*, CIE-a* )  //Quadrant by signs

if ( var_H > 0 ) var_H = ( var_H / PI ) * 180
else             var_H = 360 - ( abs( var_H ) / PI ) * 180

CIE-L* = CIE-L*
CIE-C* = sqrt( CIE-a* ^ 2 + CIE-b* ^ 2 )
CIE-HЎ = var_H

//------------------------------
//
// CIE-L*CH -> CIE-L*ab
//
//------------------------------

//Where CIE-HЎ = 0 Ц 360Ў

CIE-L* = CIE-L*
CIE-a* = cos( degree_2_radian( CIE-HЎ ) ) * CIE-C*
CIE-b* = sin( degree_2_radian( CIE-HЎ ) ) * CIE-C*

//------------------------------
//
// XYZ -> CIE-L*uv
//
//------------------------------
var_U = ( 4 * X ) / ( X + ( 15 * Y ) + ( 3 * Z ) )
var_V = ( 9 * Y ) / ( X + ( 15 * Y ) + ( 3 * Z ) )

var_Y = Y / 100
if ( var_Y > 0.008856 ) var_Y = var_Y ^ ( 1/3 )
else                    var_Y = ( 7.787 * var_Y ) + ( 16 / 116 )

ref_X =  95.047        //Observer= 2Ў, Illuminant= D65
ref_Y = 100.000
ref_Z = 108.883

ref_U = ( 4 * ref_X ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )
ref_V = ( 9 * ref_Y ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )

CIE-L* = ( 116 * var_Y ) - 16
CIE-u* = 13 * CIE-L* * ( var_U - ref_U )
CIE-v* = 13 * CIE-L* * ( var_V - ref_V )

//------------------------------
//
// CIE-L*uv -> XYZ
//
//------------------------------

var_Y = ( CIE-L* + 16 ) / 116
if ( var_Y^3 > 0.008856 ) var_Y = var_Y^3
else                      var_Y = ( var_Y - 16 / 116 ) / 7.787

ref_X =  95.047      //Observer= 2Ў, Illuminant= D65
ref_Y = 100.000
ref_Z = 108.883

ref_U = ( 4 * ref_X ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )
ref_V = ( 9 * ref_Y ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )

var_U = CIE-u* / ( 13 * CIE-L* ) + ref_U
var_V = CIE-v* / ( 13 * CIE-L* ) + ref_V

Y = var_Y * 100
X =  - ( 9 * Y * var_U ) / ( ( var_U - 4 ) * var_V  - var_U * var_V )
Z = ( 9 * Y - ( 15 * var_V * Y ) - ( var_V * X ) ) / ( 3 * var_V )

//------------------------------
//
// RGB -> HSL
//
//------------------------------

var_R = ( R / 255 )                     //Where RGB values = 0 Ц 255
var_G = ( G / 255 )
var_B = ( B / 255 )

var_Min = min( var_R, var_G, var_B )    //Min. value of RGB
var_Max = max( var_R, var_G, var_B )    //Max. value of RGB
del_Max = var_Max - var_Min             //Delta RGB value

L = ( var_Max + var_Min ) / 2

if ( del_Max == 0 )                     //This is a gray, no chroma...
{
   H = 0                                //HSL results = 0 Ц 1
   S = 0
}
else                                    //Chromatic data...
{
   if ( L < 0.5 ) S = del_Max / ( var_Max + var_Min )
   else           S = del_Max / ( 2 - var_Max - var_Min )

   del_R = ( ( ( var_Max - var_R ) / 6 ) + ( del_Max / 2 ) ) / del_Max
   del_G = ( ( ( var_Max - var_G ) / 6 ) + ( del_Max / 2 ) ) / del_Max
   del_B = ( ( ( var_Max - var_B ) / 6 ) + ( del_Max / 2 ) ) / del_Max

   if      ( var_R == var_Max ) H = del_B - del_G
   else if ( var_G == var_Max ) H = ( 1 / 3 ) + del_R - del_B
   else if ( var_B == var_Max ) H = ( 2 / 3 ) + del_G - del_R

   if ( H < 0 ) ; H += 1
   if ( H > 1 ) ; H -= 1
}

//------------------------------
//
// HSL -> RGB
//
//------------------------------

if ( S == 0 )                       //HSL values = 0 Ц 1
{
   R = L * 255                      //RGB results = 0 Ц 255
   G = L * 255
   B = L * 255
}
else
{
   if ( L < 0.5 ) var_2 = L * ( 1 + S )
   else           var_2 = ( L + S ) - ( S * L )

   var_1 = 2 * L - var_2

   R = 255 * Hue_2_RGB( var_1, var_2, H + ( 1 / 3 ) )
   G = 255 * Hue_2_RGB( var_1, var_2, H )
   B = 255 * Hue_2_RGB( var_1, var_2, H - ( 1 / 3 ) )
}

Hue_2_RGB( v1, v2, vH )             //Function Hue_2_RGB
{
   if ( vH < 0 ) vH += 1
   if ( vH > 1 ) vH -= 1
   if ( ( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6 * vH )
   if ( ( 2 * vH ) < 1 ) return ( v2 )
   if ( ( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( 2 / 3 ) - vH ) * 6 )
   return ( v1 )
}

//------------------------------
//
// RGB -> HSV
//
//------------------------------

var_R = ( R / 255 )                     //RGB values = 0 Ц 255
var_G = ( G / 255 )
var_B = ( B / 255 )

var_Min = min( var_R, var_G, var_B )    //Min. value of RGB
var_Max = max( var_R, var_G, var_B )    //Max. value of RGB
del_Max = var_Max - var_Min             //Delta RGB value

V = var_Max

if ( del_Max == 0 )                     //This is a gray, no chroma...
{
   H = 0                                //HSV results = 0 Ц 1
   S = 0
}
else                                    //Chromatic data...
{
   S = del_Max / var_Max

   del_R = ( ( ( var_Max - var_R ) / 6 ) + ( del_Max / 2 ) ) / del_Max
   del_G = ( ( ( var_Max - var_G ) / 6 ) + ( del_Max / 2 ) ) / del_Max
   del_B = ( ( ( var_Max - var_B ) / 6 ) + ( del_Max / 2 ) ) / del_Max

   if      ( var_R == var_Max ) H = del_B - del_G
   else if ( var_G == var_Max ) H = ( 1 / 3 ) + del_R - del_B
   else if ( var_B == var_Max ) H = ( 2 / 3 ) + del_G - del_R

   if ( H < 0 ) ; H += 1
   if ( H > 1 ) ; H -= 1
}

//------------------------------
//
// HSV -> RGB
//
//------------------------------

if ( S == 0 )                       //HSV values = 0 Ц 1
{
   R = V * 255
   G = V * 255
   B = V * 255
}
else
{
   var_h = H * 6
   if ( var_h == 6 ) var_h = 0      //H must be < 1
   var_i = int( var_h )             //Or ... var_i = floor( var_h )
   var_1 = V * ( 1 - S )
   var_2 = V * ( 1 - S * ( var_h - var_i ) )
   var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) )

   if      ( var_i == 0 ) { var_r = V     ; var_g = var_3 ; var_b = var_1 }
   else if ( var_i == 1 ) { var_r = var_2 ; var_g = V     ; var_b = var_1 }
   else if ( var_i == 2 ) { var_r = var_1 ; var_g = V     ; var_b = var_3 }
   else if ( var_i == 3 ) { var_r = var_1 ; var_g = var_2 ; var_b = V     }
   else if ( var_i == 4 ) { var_r = var_3 ; var_g = var_1 ; var_b = V     }
   else                   { var_r = V     ; var_g = var_1 ; var_b = var_2 }

   R = var_r * 255                  //RGB results = 0 Ц 255
   G = var_g * 255
   B = var_b * 255
}



#endif