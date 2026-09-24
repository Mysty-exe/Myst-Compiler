struct Vector2:
    int x
    int y

func main():
    Vector2 pos = Vector2(10, 20)
    pos.x = pos.x + 5
    
    print(pos.x)
    print(pos.y)