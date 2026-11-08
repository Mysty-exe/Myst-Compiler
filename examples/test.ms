container Person:
    int age, year, date
    char test, testing

    Person(int a, int b):
        age = a
        year = b

    getAge() -> int:
        return age

    getYear() -> int:
        return year

func getNumber() -> int:
    return 10

func test(int x):
    x = toInt('c') ^ 2

Person y = 34
int b = 45
test(y)
print(y)
