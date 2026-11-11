container Person:
    int id
    int age
    decimal salary

func main() -> int:
    Person p = Person(101, 28, 75000.50)
    p.age = 29
    print(p.age)
    print(p.salary)
    return 0

main()
