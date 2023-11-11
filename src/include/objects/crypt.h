class Crypt
{
public:
    Crypt() {}
    ~Crypt() {}

    string d(string o)
    {
        return decrypt(o);
    }

    string e(string o)
    {
        return encrypt(o);
    }

private:
    string decrypt(string o)
    {
        int l = o.length(), s = 7;
        string ax("");

        for (int i = 0; i < l; i++)
        {
            if (s == 7)
            {
                ax.push_back(((char)((int)o[i] + 3)));
                s = 5;
            }
            else if (s == 5)
            {
                ax.push_back(((char)((int)o[i] - 1)));
                s = 0;
            }
            else if (s == 0)
            {
                ax.push_back(((char)((int)o[i] + 4)));
                s = 1;
            }
            else
            {
                ax.push_back(((char)((int)o[i] - 2)));
                s = 7;
            }
        }

        return ax;
    }

    string encrypt(string o)
    {
        int l = o.length(), s = 7;
        string ax("");

        for (int i = 0; i < l; i++)
        {
            if (s == 7)
            {
                ax.push_back(((char)((int)o[i] - 3)));
                s = 5;
            }
            else if (s == 5)
            {
                ax.push_back(((char)((int)o[i] + 1)));
                s = 0;
            }
            else if (s == 0)
            {
                ax.push_back(((char)((int)o[i] - 4)));
                s = 1;
            }
            else
            {
                ax.push_back(((char)((int)o[i] + 2)));
                s = 7;
            }
        }

        return ax;
    }
};