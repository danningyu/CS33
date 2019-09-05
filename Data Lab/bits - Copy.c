/* 
 * CS:APP Data Lab 
 * 
 * <Danning Yu, 305087992>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif

/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
	  //use DeMorgan's Law
	  return ~((~x) | (~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (least significant) to 3 (most significant)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
	//8 bits per byte, so shift amount = 8*n...
	//..then shift desired byte all the way to the right...
	//...and then mask off everything else
	return (x>> (n<<3) ) & 0xFF;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
	//shift x and then mask off the extraneous bits
	//mask: need a reverse mask: n 0's followed by all 1's
	//thus, make a mask and then reverse it, so we want n 1's followed by 0's
	//first, make 0's: 0x1 << 31 -> 10000...000
	//then get the n 1's: (0x1 << 31) >> n -> 11...1100...00
	//but an extra one b/c we already have one to start with, so shift it back left
	//then reverse the mask and use it as a mask
	int isolated = (x >> n);
	int preMask = ((0x1 << 31) >> n) << 1;
	int mask = ~preMask;
	return isolated & mask;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
	int mask1 = 0x55 + (0x55 << 8) + (0x55 << 16) + (0x55 << 24); //0101010101010
	int mask2 = 0x33 + (0x33 << 8) + (0x33 << 16) + (0x33 << 24); //001100110011
	int mask3 = 0x0F + (0x0F << 8) + (0x0F << 16) + (0x0F << 24); //0000111100001111
	int mask4 = 0xFF + (0xFF << 16); //8 0's, 8 1's, 8 0's, 8 1's
	int mask5 = 0xFF + (0xFF << 8); //16 0's, 16 1's
	int total = (x&mask1) + ((x >> 1)&mask1); //get number of bits, taken 2 at a time
	total = (total&mask2) + ((total >> 2)&mask2);
	total = (total + (total >> 4))&mask3;
	total = (total + (total >> 8))&mask4;
	total = (total + (total >> 16))&mask5;
	return total;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
	//if x is not 0, then x|-x will give MSB of 1
	//otherwise, if x = 0, then x|-x gives MSB of 0
	//copy the MSB using right shift
	//11..111 + 1 overflows over to all 0's
	//then add 1 to get 1, as desired
	int containsMSB = x | (~x + 1);
	return (containsMSB >> 31) + 1;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
	//same strategy as bang, but use the result as a mask
	//if x=0, get all 0's, otherwise get all 1's:
		//to do this, strip info away from x and then use overflow
	//if 0000, then return z: z&~mask, and kill off y
	//if 1111, then return y: z&~mask kills z and just return y
	int zeroOrOne = !!x;
	int mask = ~zeroOrOne + 1; //take advantage of overflow
	return (y & mask) + (z & (~mask));
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int isPower2(int x) { //DONE?
	int single1 = !((x + (~0)) & x); //1 if there's only one 1, 0 otherwise
	int temp = x & (1 << 31); //if x<0, then 1000000, //otherwise, 0000000
	int isNegative = !(temp >> 31); //111111 for x<0, else 00000 for x>=0
	//then negate it...0 for x<0, >=0 for 1
	//need special case for x = 0
	return single1 & isNegative & !!x;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
	//want mask of 101010101010...alternate
	int mask = (0xAA << 8) + 0xAA;
	mask = (mask << 16) + mask;
	int oddsOnly = x & mask;
	int temp = oddsOnly ^ mask;
	return !temp;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
	//use a==b can be written as !(a^b)
	//thus, a is original, b is a expressed with only n bits
	int shiftAmount = 32 + ~n + 1; //32-n
	int trunc = (x << shiftAmount) >> shiftAmount;
	return !(x^trunc);
}
/* 
 * dividePower2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: dividePower2(15,1) = 7, dividePower2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int dividePower2(int x, int n) {
	int negOrZero = x >>( (32 << 3) + ~0);
	int bias = negOrZero & ((1 << n) + ~0);
	return (x + bias) >> n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  //use the formula -x = ~x + 1
	return (~x) + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int isPositive(int x) { //DONE
	int temp = x & (1 << 31); //if x<0, then 1000000
	//otherwise, 0000000
	return !((temp >> 31) | !x); //111111 for x<0, else 00000 for x>=0
	//!x = 000001 for x = 0, =00000 otherwise -> the | gives an extra 1 for x = 0 case...
	//then invert again: !00001 = 0, !00000 = 1, !11111 = 0, check
	//if x<0, then returns 0; if x>=0, then returns 1 ->ERROR for 0
	//|!x to handle special case of x=0
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) { //DONE?
	int xNegative = (x >> 31) & 0x1; //1 if x<0
	int yNegative = (y >> 31) & 0x1; //1 if y<0
	int differentSigns = xNegative ^ yNegative; //1 if x and y have opposite signs
	int case1 = differentSigns & xNegative; //if x is negative and y is positive, MUST be smaller
	//else:
	int difference = (~x) + y + 1;
	int diffNegative = (difference >> 31); //only preserve sign bit
	int b = !differentSigns & !diffNegative; //same signs and difference is pos or = 0, gives 1
	return b| case1;


	int diff_sgn = !(x >> 31) ^ !(y >> 31);      //is 1 when signs are different
	int a = diff_sgn & (x >> 31);            //diff signs and x is neg, gives 1
	
	int f = a | b;
	return f;
	//if y>x, then temp<0 and return 1, otherwise, if x<=y, then temp>=0 and return 0
	//thus, we want isPosOrZero(int temp): if temp<0, return 0, otherwise, if temp>=0, return 1
	//int diffNeg = ((x + (~y + 1)) >> 31); //1 if x-y is negative, 0 otherwise
	//if x-y is nonnegative BUT x is negative, then overflow occurred...
	//return (!diffNeg&overflowPossible)| (xNegative & (!overflowPossible));
		//return if the difference is negative (if so, then y>x, so return 0
	//however, also check if overflow occurs: if x is negative and overflow can occur, did it occur? If so then x must be smaller than y
}
/*
 * intLog2 - return floor(log base 2 of x), where x > 0
 *   Example: intLog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int intLog2(int x) {
	int high16NonZero = (!!(x >> 16));
	int result = high16NonZero << 4;
	int high8NonZero = !!(x >> (8 + result));
	result = result + (high8NonZero << 3);
	int high4NonZero = !!(x >> (4 + result));
	result = result + (high4NonZero << 2);
	int high2NonZero = !!(x >> (2 + result));
	result = result + (high2NonZero << 1);
	int nonZero = !!(x >> (1 + result));
	result = result + nonZero;
	return result;
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2 
 */
int leastBitPos(int x) {
	return (x & (~x + 1));
}
