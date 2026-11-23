func gradeTier(int score, decimal multiplier, bool waived) -> char:
    if (score >= 90 && multiplier >= 1.0) || waived:
        return 'A'
    elseif score >= 75 && !(multiplier < 0.8):
        return 'B'
    else:
        return 'C'

int baseScore = 82
decimal mult = 1.25
bool isWaived = false
char grade = 'c'

if gradeTier(baseScore, mult, isWaived) == 'B':
    print(343)
