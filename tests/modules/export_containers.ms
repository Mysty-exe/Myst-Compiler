export { Vector, addVectors }

container Vector:
    int x
    int y

func addVectors(Vector a, Vector b) -> Vector:
    return Vector(a.x + b.x, a.y + b.y)