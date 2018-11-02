#pragma once

/**
 * 数据库表一列的单元属性
 */
struct oci_column
{
    string      strColumnName;  //< 列名称
    uint32_t    nColumnType;    //< 列类型
    uint32_t    nMaxLength;     //< 最大长度，类型是字符串时必须设置
    bool        bNullable;      //< 是否可为空
    string      strDefault;     //< 默认值，偶尔需要
};

/**
 * 封装Oracle插入数据的操作
 */
class OracleInsert
{
public:
    OracleInsert();
    OracleInsert(string strDB, string strTable, int nRowSize = 100);
    ~OracleInsert(void);

    /**
     * 初始化数据库配置
     * @param strDB 数据配置名字
     * @param strTable 表名字
     * @param nRowSize 一次提交的数据行数
     */
    void Init(string strDB, string strTable, int nRowSize);

    /**
     * 数据库增加一个字段
     * @param strColumnName 列名称
     * @param nColumnType 类型
     * @param nMaxLength 类型为字符串时，需要设置长度
     * @param bNullable 当允许为空时，插入的数据内容如果为空，则将字段设置为空
     * @param strDefault 默认值，当不允许为空，且插入的数据内容为空，则将字段设为默认值
     */
    void AddCloumn(string strColumnName, 
        uint32_t nColumnType, 
        uint32_t nMaxLength = 16, 
        bool bNullable = false, 
        string strDefault = "");

    /**
     * 预处理，必须在AddCloumn结束后使用
     * 生成sql，申请存放数据的空间
     */
    bool Prepair();

    /**
     * 批量插入数据
     * @param rows 多行数据，每行的字段个数应该与设置的相同
     */
    bool Insert(vector<vector<string>> rows);

private:
    //需要保证target空间足够, len为不包含'\0'的长度值
    static void str_set(char *target, int index, int len, const char *source);
    static void bind_set_data(OCI_Bind* bind, int index, int len, string val, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, string val, string date_fmt, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, int16_t val, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, int32_t val, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, int64_t val, bool is_null = false);

private:
    string                  m_strDataBase;      //< 数据库配置名称
    string                  m_strTableName;     //< 要插入的表名称
    vector<oci_column>      m_vecTableRow;      //< 表定义

    char*                   m_buff;             //< 内存区域
    int                     m_nPointLen;        //< 指针的长度，32位平台4字节，64位平台8字节
    int                     m_nRowSize;         //< 一次提交的数据量
    string                  m_strSql;           //< 执行插入的sql语句
};

