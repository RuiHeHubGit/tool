//
// Created by shuihe on 2022/9/9.
//

#include "levenshtein.h"

int Min(int x, int y, int z)
{
    if(x <= y && x <= z)
        return x;
    else if (y <= z)
        return y;
    else
        return z;
}

int levenshteinTwoRows(char s[], int s_len, char t[], int t_len)
{
    //退化的基本情况
    if (s_len == 0)
        return t_len;
    if (t_len == 0)
        return s_len;
    //构造两个工作向量，存放编辑距离的当前行和前一行
    int v0[t_len + 1], v1[t_len + 1];
    int i, j;
    //初始化v0，即是A[0][i]，从空字符串s到目标字符串t，只要添加每个字符
    for (i = 0; i <= t_len; i++)
        v0[i] = i;
    for (i = 0; i < s_len; i++) {
        //从前一行v0计算v1，v1的第一个元素是A[i+1][0],
        //编辑距离就是从原字符串s中删除每个字符到目标字符串t
        v1[0] = i + 1;
        for (j = 0; j < t_len; j++) {
            int cost = (s[i] == t[j]) ? 0:1;
            v1[j + 1] = Min(v1[j] + 1, v0[j + 1] + 1, v0[j] + cost);
        }

        //为了下一次迭代，复制v1到v0
        for (j = 0; j <= t_len; j++)
            v0[j] = v1[j];
    }
    return v1[t_len];
}