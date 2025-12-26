#ifndef AUTH_LINE_EDIT_H
#define AUTH_LINE_EDIT_H

#include <QLineEdit>
#include <QPushButton>
#include <QResizeEvent>

class AuthLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit AuthLineEdit(const QString& placeholder, QWidget* parent = nullptr);
    
    // 设置为密码模式（显示切换按钮）
    void setPasswordMode(bool enable);
    
protected:
    // 重写resize事件以更新按钮位置
    void resizeEvent(QResizeEvent* event) override;
    
private slots:
    void togglePasswordVisibility();
    
private:
    QPushButton* toggleButton;  // 密码显示/隐藏切换按钮
    bool isPasswordVisible;     // 密码是否可见
    bool isPasswordMode;        // 是否为密码模式
    
    void setupToggleButton();
    void updateToggleButtonIcon();
};

#endif // AUTH_LINE_EDIT_H

