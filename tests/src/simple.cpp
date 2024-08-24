struct Test
{
    int a;
    int b;
    int c;
};

int Compute()
{
    Test t;
    t.a = 1;
    t.b = 2;
    t.c = 3;
    return t.a + t.b + t.c;
}

int main()
{
    return Compute();
}