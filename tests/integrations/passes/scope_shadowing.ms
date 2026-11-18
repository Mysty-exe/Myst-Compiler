func main() -> int:
    // 1. Outermost scope variable
    int x = 100
    print(x)

    // 2. First level of nesting (shadows outer x)
    if x > 50:
        int x = 200
        print(x)
        
        // 3. Second level of nesting (shadows middle x)
        if x == 200:
            int x = 300
            print(x)
        
        // Should pop back to the middle scope's x (200)
        print(x)
    
    // Should pop all the way back to the outermost scope's x (100)
    print(x)

    // 4. Testing scope blocks inside a while loop
    int i = 0
    while i < 2:
        int loop_local = i * 50
        print(loop_local)
        i = i + 1

    return 0

main()