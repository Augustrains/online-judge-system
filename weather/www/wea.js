<template>
  <div class="weather_font">
    <span v-if="loading">加载中...</span>
    <span v-else-if="error">{{ error }}</span>
    <span v-else>{{ weather }}&nbsp;&nbsp;</span>
    <span v-else>{{ temp }}℃&nbsp;&nbsp;</span>
    <span v-else>{{ wind }}</span>
  </div>
</template>

<script>
export default {
  data() {
    return {
      weather: '',
      temp: '',
      wind: '',
      loading: true,
      error: null
    };
  },
  created() {
    this.getData();
  },
  methods: {
    async getData() {
      try {
        // 心知天气API配置
        const key = "MJgPiYpEd1ndzXJQZUVZTH9BZ7inlw4o"; // 心知天气key
        const location = "101060101"; // 雷波的拼音或ID
        const url = `https://api.seniverse.com/v3/weather/now.json?key=${key}&location=${location}&language=zh-Hans&unit=c`;

        const response = await fetch(url);
        const data = await response.json();

        // 检查API响应是否成功
        if (data.status === 'ok' && data.results && data.results.length > 0) {
          const result = data.results[0].now;
          this.weather = result.text;
          this.temp = result.temperature;
          this.wind = `${result.wind_direction} ${result.wind_scale}级`;
        } else {
          throw new Error(data.status || '获取天气数据失败');
        }
      } catch (error) {
        console.error('获取天气数据出错:', error);
        this.error = '天气数据获取失败，请稍后重试';
      } finally {
        this.loading = false;
      }
    }
  }
};
</script>

<style>
.weather_font {
  font-size: 14px;
  color: #32F0EE;
}
</style>