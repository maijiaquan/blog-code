class Solution {
public:
    int largestRectangleArea(vector<int>& heights) {
        int n = heights.size();
        if(n <= 0) return 0;
        int res = 0;
        
        int leftCorners[n]; //leftCorners[i]表示i所能围成的最大矩形的左边界
        int RightCorners[n];    //RightCorners[i]表示i所能围成的最大矩形的右边界

        RightCorners[n-1] = n; //初始化最后一个的右边界
        leftCorners[0] = -1;   //初始化第一个的左边界
        
        for(int i = 1; i < n; i++)
        {
            int left = i - 1;
            while(left >= 0 && heights[left] >= heights[i]) 
                left = leftCorners[left];
            leftCorners[i] = left;
        }
        
        for(int i = n-2; i >= 0; i--)
        {
            int right = i + 1;
            while(right < n && heights[right] >= heights[i])
                right = RightCorners[right];
            RightCorners[i] = right;
        }
        
        for(int i = 0; i < n; i++)
        {
            res = max(res, heights[i]*(RightCorners[i] - leftCorners[i] -1));
        }
        
        return res;
    }
};