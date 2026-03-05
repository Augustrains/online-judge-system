from sparkai.llm.llm import ChatSparkLLM, ChunkPrintHandler
from sparkai.core.messages import ChatMessage

# 星火认知大模型配置
SPARKAI_URL = 'wss://spark-api.xf-yun.com/v4.0/chat'
SPARKAI_APP_ID = '7606551a'
SPARKAI_API_SECRET = 'OTY0Y2E1ZDc5NDhiNGZjNGUyYjExYjY1'
SPARKAI_API_KEY = 'ee22d580bf5278a6ad427e35e33bbcf4'
SPARKAI_DOMAIN = '4.0Ultra'

# 文件路径配置
INPUT_FILE = '/root/OnlineJudge/oj_server/what.txt'  # 输入文件路径
OUTPUT_FILE = '/root/OnlineJudge/oj_server/answer.txt'  # 输出文件路径

if __name__ == '__main__':
    try:
        # 1. 从文件读取输入内容
        with open(INPUT_FILE, 'r', encoding='utf-8') as f:
            input_content = f.read().strip()
            print(f"已读取输入文件内容（长度: {len(input_content)} 字符）")
        
        # 2. 初始化模型
        spark = ChatSparkLLM(
            spark_api_url=SPARKAI_URL,
            spark_app_id=SPARKAI_APP_ID,
            spark_api_key=SPARKAI_API_KEY,
            spark_api_secret=SPARKAI_API_SECRET,
            spark_llm_domain=SPARKAI_DOMAIN,
            streaming=False,
        )
        
        # 3. 构建请求消息
        messages = [ChatMessage(
            role="user",
            content=input_content  # 使用文件内容作为输入
        )]
        
        # 4. 调用模型（保留回调处理器，用于打印中间结果）
        handler = ChunkPrintHandler()
        response = spark.generate([messages], callbacks=[handler])
        
        # 5. 提取回复内容
        if response and response.generations:
            answer = response.generations[0][0].text
            print(f"模型回复（长度: {len(answer)} 字符）")
            
            # 6. 将回复写入输出文件
            with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
                f.write(answer)
                print(f"已将回复保存到 {OUTPUT_FILE}")
        else:
            print("错误：未收到有效回复")
            
    except FileNotFoundError:
        print(f"错误：找不到输入文件 {INPUT_FILE}")
    except Exception as e:
        print(f"发生异常：{e}")