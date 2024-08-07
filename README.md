# Pomodoro Timer

## Изображение

![image](https://github.com/user-attachments/assets/e0875028-eec4-49d5-b66b-a16f0584d0c7) <br> Рисунок 1. Интерфейс программы

## Описание

Pomodoro Timer — это простое приложение для таймера техники Помодоро на Windows. Оно использует стандартный таймер Помодоро с возможностью настройки времени работы и перерыва. Приложение отображает оставшееся время и уведомляет пользователя о смене режимов работы и перерыва.

## Функции

- Запуск и остановка таймера Помодоро.
- Настройка времени работы и времени перерыва.
- Уведомления о переходе в режим перерыва или работы.
- Интеграция с системным трей.

## Сборка

Для сборки проекта вам понадобится установленный [GCC](https://gcc.gnu.org/) и [MinGW](https://www.mingw-w64.org/downloads/ (если вы работаете на Windows).

1. **Установите необходимые инструменты** (GCC и MinGW).

2. **Создайте исполняемый файл**:

   Откройте командную строку и перейдите в каталог, содержащий ваш исходный код и `Makefile`. Выполните команду:

   ```bash
   make
   ```

   Это соберет проект и создаст исполняемый файл `pomodoro.exe`.

## Использование

1. **Запустите приложение** `pomodoro.exe`.

2. **Настройте время работы и перерыва**:

   - Введите желаемое время работы в минутах в поле "Pomodoro (min):".
   - Введите желаемое время перерыва в минутах в поле "Break (min):".

3. **Запустите или остановите таймер** с помощью кнопки "Start" или "Stop".

4. **Отображение в системном трее**:

   Приложение также отображается в системном трее, где вы можете:
   - Восстановить окно приложения (правый клик на иконке и выберите "Show").
   - Закрыть приложение (правый клик на иконке и выберите "Exit").

## Очистка

Для очистки собранных файлов выполните команду:

```bash
make clean
```

Это удалит объектные файлы и исполняемый файл.

## Примечания

- При возникновении проблем с компиляцией или запуском, убедитесь, что все зависимости установлены правильно.

## Изменения версий
- **v1.0**: Первая версия с базовыми функциями и интеграцией с системным треем.
- **v1.0.1**: Улучшен UI интерфейс.
- **v1.1.1**: Изменен интерфейс программы на темную тему и изменен дизайн элементов.
- **v1.1.2**: Добавлена цветная реакция на клик по кнопке.

## Лицензия

Этот проект является открытым и свободно распространяемым по лицензия **MIT**. Вы можете использовать и модифицировать его в соответствии с вашими потребностями.

---

Разработано [Tailogs](https://github.com/tailogs).
