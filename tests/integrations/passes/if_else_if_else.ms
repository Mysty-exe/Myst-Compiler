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

if gradeTier(baseScore, mult, isWaived) == 'B' && baseScore > 80:
    if gradeTier(95, 0.5, false) == 'A':
        print('1')
    else:
        print('0')
elseif gradeTier(50, 1.0, true) == 'A':
    print('2')
else:
    print('3')

if gradeTier(91, 1.1, false) == 'A' && !isWaived:
    print('4')
elseif gradeTier(70, 0.5, false) == 'B':
    print('5')
else:
    print('6')

if gradeTier(45, 0.5, false) == 'A':
    print('7')
elseif gradeTier(45, 0.5, false) == 'B':
    print('8')
else:
    print('9')