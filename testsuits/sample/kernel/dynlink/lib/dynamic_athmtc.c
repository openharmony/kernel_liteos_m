#define OK          0
#define NOK         1

void It_dynlink_dowhile(int cnt, int *out)
{
    int outParam = 0;
    cnt += 1;

    do {
        ++outParam;
        --cnt;
    } while (cnt);

    *out = outParam;
}

void It_dynlink_while(int cnt, int *out)
{
    int index = 0;
    int outParam = 0;
    cnt += 1;

    while ((index++) < cnt) {
        outParam += 1;
    }

    *out = outParam;
}

void It_dynlink_for(int cnt, int *out)
{
    int index = 0;
    int outParam = 0;
    cnt += 1;

    for (; index < cnt; ++index) {
        ++outParam;
    }

    *out = outParam;
}

void It_dynlink_ifelse(int inVal, int *outVal)
{
    int outParam;

    if (inVal <= 101) {
        outParam = 101;
    } else if (inVal == 102) {
        outParam = inVal;
    } else if (inVal == 103) {
        outParam = inVal;
    } else if (inVal == 104) {
        outParam = inVal;
    } else if (inVal == 105) {
        outParam = inVal;
    } else if (inVal == 106) {
        outParam = inVal;
    } else if (inVal == 107) {
        outParam = inVal;
    } else if (inVal == 108) {
        outParam = inVal + 1;
    } else if (inVal == 109) {
        outParam = 45;
    } else {
        outParam = 45;
    }

    *outVal = outParam;
}

void It_dynlink_continue(int cnt, int *out)
{
    int index = 0;
    int outParam = 0;
    cnt += 2;

    for (; index < cnt; ++index) {
        if (index % 2) {
            continue;
        } else {
            ++outParam;
        }
    }

    *out = outParam;
}

void It_dynlink_switch(char *inVal, char *outVal)
{
    int i;
    char outParam;

    for (i = 0; i < 3; ++i) {
        switch (inVal[i]) {
            case 'A':
            case 'a':
                outParam = 'b';
                break;
            case 'B':
            case 'b':
                outParam = 'c';
                break;
            case 'C':
            case 'c':
                outParam = 'd';
                break;
            case 'D':
            case 'd':
                outParam = 'e';
                break;
            case 'E':
            case 'e':
                outParam = 'f';
                break;
            case 'F':
            case 'f':
                outParam = 'g';
                break;
            case 'G':
            case 'g':
                outParam = 'h';
                break;
            case 'H':
            case 'h':
                outParam = 'i';
                break;
            default:
                outParam = 'z';
                break;
        }
        outVal[i] = outParam;
    }
}
