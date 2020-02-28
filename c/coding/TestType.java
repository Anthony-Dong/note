import java.util.HashMap;

public class TestType {
    public static void main(String[] args) {
        HashMap<String, String> map = new HashMap<>();
        map.put("a", 1);
        String a = map.get("a");
        System.out.println(a);
    }
}