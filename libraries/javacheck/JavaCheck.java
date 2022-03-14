public class JavaCheck
{
    private static final String[] keys = {"os.arch", "java.version", "java.vendor"};
    public static void main (String [] args)
    {
        boolean error = false;
        for(String key : keys)
        {
            String property = System.getProperty(key);
            if(property != null)
            {
                System.out.println(key + "=" + property);
            }
            else
            {
                error = true;
                break;
            }
        }
        
        if (error) {
            System.exit(1);
        }
    }
}
