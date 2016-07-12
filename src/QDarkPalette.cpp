// Style copied From: https://gist.githubusercontent.com/QuantumCD/6245215/raw/e74b6af3561bd76f7b92b8ee3c71064dfbdde43c/Qt%25205%2520Dark%2520Fusion%2520Palette
void setFusionStyle() {
	qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

	darkPalette.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);
	darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::gray);
	darkPalette.setColor(QPalette::Disabled, QPalette::BrightText, Qt::gray);
	darkPalette.setColor(QPalette::Disabled, QPalette::Link, Qt::darkGray);
	darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::gray);
    
    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}