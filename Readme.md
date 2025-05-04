# Сохранение файла
curl -X PUT http://localhost:4000/myfile -d "hello"

# Получение содержимого
curl -X GET http://localhost:4000/myfile

# Удаление файла
curl -X DELETE http://localhost:4000/myfile
