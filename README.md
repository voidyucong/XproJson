# XproJson
一个基于 C 的 JSON 解析器。

## 用法
只需要包含头文件 `xproJson.h`，例如要创建`["json", {"name":"value"}]`：

```c
XJson* array = xpro_create_array();
xpro_addStringToArray(array, "json");
XJson* object = xpro_create_object();
xpro_addStringToObject(object, "value", "name");
xpro_addItemToArray(array, object);
printf("%s\n", xpro_print(array));
xpro_free(array);
```

现在的异常提醒是个shit!!!待优化...
