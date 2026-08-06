using System;
using System.Text.RegularExpressions;
class Program {
    static void Main() {
        string url1 = "https://www.moddb.com/downloads/mirror/263531/133/6da5a9";
        string url2 = "https://www.moddb.com/downloads/mirror/123456";
        Match m1 = Regex.Match(url1, @"/downloads/mirror/(\d+)");
        Console.WriteLine(m1.Success ? m1.Groups[1].Value : "fail");
        Match m2 = Regex.Match(url2, @"/downloads/mirror/(\d+)");
        Console.WriteLine(m2.Success ? m2.Groups[1].Value : "fail");
    }
}
