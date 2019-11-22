
/* A simple java test to generate some activity to verify operf is working
 * properly. */

public class Fibonacci {
   public static void main(String[] args) {
      long sum=0;
      int i = 1;
      /* The test harness does not easily handle passing arguments
       * to the test, so hardcode a value here.
       * This (sum) will overflow, but that is fine as we are only after
       * activity, and are not concerned with the result.  */
      int n = 12345; /*Integer.parseInt(args[0]);*/
      long f1 = 0;
      long f2 = f1 + 1;
      while (i <= n) {
         sum = f1 + f2;
         f1 = f2;
         f2 = sum;
         i++;
      }
      System.out.print(sum);
      System.out.print("\n");
     }
}
