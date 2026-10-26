using System.Text;

class Program
{
    static void Main(string[] args)
    {
        int[,] test = { { 2, 5 }, { 3, 7 }, { 1, 4 } };
        Program prog = new Program();
        Console.WriteLine(prog.CompressString("aabbbccccc"));
    }

    public int[] FindAlCrossing(int[,] intervals)
    {
        if (intervals.GetLength(0) == 0) return new int[] { -1, -1 };

        int left = int.MinValue;
        int right = int.MaxValue;

        for (int i = 0; i < intervals.GetLength(0); i++)
        {

            if (intervals[i, 0] > left)
            {
                left = intervals[i, 0];
            }

            if (intervals[i, 1] < right)
            {
                right = intervals[i, 1];
            }

        }
        if (left > right)
        {
            return new int[] { -1, -1 };
        }
        return new int[] { left, right };
    }


    public int FindSkipNumber(int[] nums)
    {
        int waitingSum = nums.Length * (nums.Length + 1) / 2;
        int realSum = 0;

        for (int i = 0; i < nums.Length; i++)
        {
            realSum += nums[i];
        }

        return waitingSum - realSum;
    }

    public int FindSkipNumberXOR(int[] nums)
    {
        int result = nums.Length;

        for (int i = 0; i < nums.Length; i++)
        {
            result = result ^ i ^ nums[i];
        }

        return result;
    }

    public string CompressString(string str)
    {
        if (string.IsNullOrEmpty(str)) return str;

        int count = 1;
        StringBuilder stringBuilder = new StringBuilder();

        for (int i = 0; i < str.Length; i++)
        {
            if (i < str.Length - 1 && str[i] == str[i + 1])
            {
                count++;
            }
            else
            {
                stringBuilder.Append(str[i]);
                stringBuilder.Append(count);
                count = 1;
            }
        }

        return stringBuilder.ToString();
    }

    public IList<int[]> FindTriplets(int[] nums)
    {
        IList<int[]> result = new List<int[]>();

        Array.Sort(nums);

        for (int i = 0; i < nums.Length; i++)
        {
            var left = i + 1;
            var right = nums.Length - 1;

            while (left < right)
            {
                int sum = nums[i] + nums[left] + nums[right];

                if (sum == 0)
                {
                    result.Add(new int[3] { nums[i], nums[left], nums[right] });

                    left++;
                    right--;
                }
                else if(sum<0)
                {
                    left++;
                }
                else
                {
                    right--;
                }

            }


        }

        return result;
    }


    public int CoinChange(int[] coins, int amount)
    {
        int max = amount + 1;
        int[] dp = new int[amount + 1];
        Array.Fill(dp, max);

        dp[0] = 0;

        for (int i = 1; i <= amount; i++)
        {
            foreach (int coin in coins)
            {
                if (i - coin >= 0)
                {
                    dp[i] = Math.Min(dp[i], dp[i - coin] + 1);
                }
            }
        }

        return dp[amount] > amount ? -1 : dp[amount];
    }

}


/*
 Task 1: Пересечение интервалов
Условие: Дан массив интервалов [start, end]. Найдите пересечение всех интервалов (общий промежуток, который входит в каждый интервал). Если пересечения нет, верните [-1, -1].
Пример 1: [[2, 5], [3, 7], [1, 4]] → Ответ: [3, 4] (общая часть всех трёх интервалов)
Пример 2: [[1, 2], [3, 4], [5, 6]] → Ответ: [-1, -1] (нет общего промежутка)
Пример 3: [[1, 10], [2, 8], [3, 6]] → Ответ: [3, 6]
Детали:
Интервалы могут быть в любом порядке
Интервал считается включительно по обоим концам
Для пустого массива верните [-1, -1]
Task 2: Поиск пропущенного числа
Условие: Дан массив целых чисел от 0 до n, но одно число пропущено. Найдите пропущенное число.
Пример 1: [3, 0, 1] → Ответ: 2
Пример 2: [9, 6, 4, 2, 3, 5, 7, 0, 1] → Ответ: 8
Пример 3: [0] → Ответ: 1
Детали:
В массиве ровно n чисел, но диапазон от 0 до n (включительно)
Числа уникальны
Массив не отсортирован*/


/*Task 1 (10 минут)
Строка a2b3c5 после разжатия становится строкой aabbbccccc. Сожмите данную вам строку S.
Task 2 (10 минут)
Найдите триплеты в данном массиве, сумма чисел в которых равна нулю
Task 3 (10)
Имея монеты разного достоинства (вводятся), найдите минимальное число монет, необходимых, чтобы составить сумму (вводится). Любая монета может встречаться любое количество раз.
*/