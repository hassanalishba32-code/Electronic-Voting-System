#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtWidgets/QFrame>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <string>
#include <vector>

class PieChartWidget : public QWidget {
public:
    explicit PieChartWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(180, 180);
    }

    void setData(const std::vector<QString> &labels, const std::vector<int> &values) {
        labels_ = labels;
        values_ = values;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const int padding = 10;
        int side = std::min(width(), height()) - 2 * padding;
        side = std::max(side, 0);
        int x = (width() - side) / 2;
        int y = (height() - side) / 2;
        QRectF pieRect(x, y, side, side);

        int total = 0;
        for (int value : values_) {
            total += value;
        }

        if (total <= 0) {
            painter.setPen(QPen(QColor("#cbd5f5"), 2));
            painter.setBrush(QBrush(QColor("#eef2ff")));
            painter.drawEllipse(pieRect);

            painter.setPen(QColor("#6b7280"));
            painter.drawText(rect(), Qt::AlignCenter, "No votes yet");
            return;
        }

        const QColor colors[] = {
            QColor("#4f6bed"),
            QColor("#22c55e"),
            QColor("#f59e0b")
        };

        int startAngle = 0;
        for (size_t i = 0; i < values_.size(); ++i) {
            int spanAngle = static_cast<int>(360.0 * values_[i] / total);
            painter.setPen(Qt::NoPen);
            painter.setBrush(colors[i % 3]);
            painter.drawPie(pieRect, startAngle * 16, spanAngle * 16);
            startAngle += spanAngle;
        }
    }

private:
    std::vector<QString> labels_;
    std::vector<int> values_;
};

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        backend::LoadData(users_, voteCounts_);
        if (voteCounts_.size() != static_cast<size_t>(backend::kCandidateCount)) {
            voteCounts_.assign(backend::kCandidateCount, 0);
        }

        auto *tabs = new QTabWidget();
        tabs->addTab(buildRegisterTab(), "Register");
        tabs->addTab(buildVoteTab(), "Login & Vote");
        tabs->addTab(buildResultsTab(), "Admin");

        auto *container = new QWidget();
        auto *layout = new QVBoxLayout(container);
        layout->setContentsMargins(18, 18, 18, 18);
        layout->setSpacing(16);
        layout->addWidget(buildHeader());
        layout->addWidget(tabs);
        setCentralWidget(container);

        setWindowTitle("Electronic Voting System");
        resize(560, 420);

        QFont appFont("Helvetica Neue", 13);
        setFont(appFont);

        setStyleSheet(
            "QMainWindow { background-color: #f4f6fb; }"
            "QTabWidget::pane { border: 0; }"
            "QTabBar::tab { background: #e3e8f4; padding: 8px 18px; margin: 0 6px 0 0; border-radius: 10px; }"
            "QTabBar::tab:selected { background: #4f6bed; color: white; }"
            "QGroupBox { border: 1px solid #d7dbe7; border-radius: 12px; margin-top: 10px; padding: 10px; background: white; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; color: #4b5563; }"
            "QLineEdit { border: 1px solid #d1d5db; border-radius: 8px; padding: 6px 10px; background: white; color: #111827; }"
            "QLineEdit::selection { background: #c7d2fe; color: #111827; }"
            "QComboBox { border: 1px solid #d1d5db; border-radius: 8px; padding: 6px 10px; background: white; color: #111827; }"
            "QComboBox::drop-down { border: none; width: 28px; }"
            "QComboBox::down-arrow { image: none; border: none; }"
            "QComboBox QAbstractItemView { background: white; color: #111827; border: 1px solid #e5e7eb; padding: 0px; margin: 0px; outline: 0; }"
            "QComboBox QAbstractItemView::item { padding: 6px 10px; margin: 0px; background: white; }"
            "QComboBox QAbstractItemView::item:selected { background: #c7d2fe; color: #111827; }"
            "QComboBox QAbstractItemView::item:hover { background: #e0e7ff; color: #111827; }"
            "QPushButton { background-color: #4f6bed; color: white; border: none; border-radius: 10px; padding: 8px 18px; }"
            "QPushButton:hover { background-color: #4159c9; }"
            "QPushButton:disabled { background-color: #b9c2e5; color: #f8fafc; }"
            "QLabel { color: #1f2937; }"
            "QMessageBox { background-color: white; }"
            "QMessageBox QLabel { color: #111827; }"
            "QMessageBox QPushButton { background-color: #4f6bed; color: white; border: none; border-radius: 8px; padding: 6px 16px; }"
            "QMessageBox QPushButton:hover { background-color: #4159c9; }"
        );
    }

private:
    std::vector<backend::User> users_;
    std::vector<int> voteCounts_;

    QLineEdit *regCnic_ = nullptr;
    QLineEdit *regPassword_ = nullptr;

    QLineEdit *loginCnic_ = nullptr;
    QLineEdit *loginPassword_ = nullptr;
    QComboBox *candidatePicker_ = nullptr;
    QPushButton *voteButton_ = nullptr;
    int loggedInIndex_ = -1;

    QLineEdit *adminPassword_ = nullptr;
    QLabel *resultsLabel_ = nullptr;
    QGroupBox *resultsPanel_ = nullptr;
    QScrollArea *resultsScroll_ = nullptr;
    QLabel *countLabels_[backend::kCandidateCount] = {nullptr, nullptr, nullptr};
    PieChartWidget *pieChart_ = nullptr;
    QWidget *legendWidget_ = nullptr;

    QString resolveAssetPath(const QString &relativePath) {
        QDir appDir(QCoreApplication::applicationDirPath());
        QStringList candidates = {
            appDir.filePath(relativePath),
            appDir.filePath("../" + relativePath),
            QDir::current().filePath(relativePath),
            QDir::current().filePath("../" + relativePath)
        };
        for (const auto &path : candidates) {
            if (QFileInfo::exists(path)) {
                return path;
            }
        }
        return appDir.filePath(relativePath);
    }

    QWidget *buildHeader() {
        auto *header = new QWidget();
        auto *layout = new QHBoxLayout(header);
        layout->setContentsMargins(8, 6, 8, 6);
        layout->setSpacing(12);

        auto *evmLogo = new QLabel();
        QPixmap evmPixmap(resolveAssetPath("logos/evm_logo.png"));
        if (!evmPixmap.isNull()) {
            evmLogo->setPixmap(makeRoundPixmap(evmPixmap, 72));
        }
        evmLogo->setFixedSize(80, 80);
        evmLogo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        auto *title = new QLabel("Electronic Voting System");
        QFont titleFont = font();
        titleFont.setPointSize(18);
        titleFont.setBold(true);
        title->setFont(titleFont);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("color: #1f2937;");

        auto *uniLogo = new QLabel();
        QPixmap uniPixmap(resolveAssetPath("logos/uni_logo.jpeg"));
        if (!uniPixmap.isNull()) {
            uniLogo->setPixmap(makeRoundPixmap(uniPixmap, 60));
        }
        uniLogo->setFixedSize(66, 66);
        uniLogo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        uniLogo->setStyleSheet("border: 1px solid #4c5faa; border-radius: 33px;");

        layout->addWidget(evmLogo);
        layout->addWidget(title, 1);
        layout->addWidget(uniLogo);
        return header;
    }

    QPixmap makeRoundPixmap(const QPixmap &source, int size) {
        QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap output(size, size);
        output.fill(Qt::transparent);

        QPainter painter(&output);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, scaled);
        return output;
    }

    QWidget *buildRegisterTab() {
        auto *tab = new QWidget();
        auto *layout = new QVBoxLayout(tab);
        layout->setSpacing(14);

        auto *form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignLeft);
        form->setFormAlignment(Qt::AlignTop);
        regCnic_ = new QLineEdit();
        regPassword_ = new QLineEdit();
        regPassword_->setEchoMode(QLineEdit::Password);
        form->addRow("CNIC (13 digits):", regCnic_);
        form->addRow("Password:", regPassword_);

        auto *registerButton = new QPushButton("Register");
        registerButton->setMinimumHeight(36);
        connect(registerButton, &QPushButton::clicked, [this]() { handleRegister(); });

        layout->addLayout(form);
        layout->addWidget(registerButton);
        layout->addStretch();
        return tab;
    }

    QWidget *buildVoteTab() {
        auto *tab = new QWidget();
        auto *layout = new QVBoxLayout(tab);
        layout->setSpacing(14);

        auto *loginGroup = new QGroupBox("Login");
        auto *loginLayout = new QFormLayout(loginGroup);
        loginLayout->setLabelAlignment(Qt::AlignLeft);
        loginLayout->setFormAlignment(Qt::AlignTop);
        loginCnic_ = new QLineEdit();
        loginPassword_ = new QLineEdit();
        loginPassword_->setEchoMode(QLineEdit::Password);
        loginLayout->addRow("CNIC:", loginCnic_);
        loginLayout->addRow("Password:", loginPassword_);

        auto *loginButton = new QPushButton("Login");
        loginButton->setMinimumHeight(36);
        connect(loginButton, &QPushButton::clicked, [this]() { handleLogin(); });
        loginLayout->addRow(loginButton);

        auto *voteGroup = new QGroupBox("Vote");
        auto *voteLayout = new QFormLayout(voteGroup);
        voteLayout->setLabelAlignment(Qt::AlignLeft);
        voteLayout->setFormAlignment(Qt::AlignTop);
        candidatePicker_ = new QComboBox();
        auto *candidateView = new QListView(candidatePicker_);
        candidateView->setSpacing(0);
        candidateView->setContentsMargins(0, 0, 0, 0);
        candidateView->setMouseTracking(true);
        candidateView->setFrameShape(QFrame::NoFrame);
        candidateView->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        candidateView->setAttribute(Qt::WA_TranslucentBackground, false);
        candidateView->setStyleSheet(
            "QListView { background: white; color: #111827; border: 1px solid #e5e7eb; padding: 0px; margin: 0px; outline: 0; }"
            "QListView::item { padding: 6px 10px; margin: 0px; background: white; }"
            "QListView::item:selected { background: #c7d2fe; color: #111827; }"
            "QListView::item:hover { background: #e0e7ff; color: #111827; }"
        );
        candidatePicker_->setView(candidateView);
        for (int i = 0; i < backend::kCandidateCount; ++i) {
            candidatePicker_->addItem(backend::kCandidates[i], i);
        }
        voteButton_ = new QPushButton("Cast Vote");
        voteButton_->setEnabled(false);
        voteButton_->setMinimumHeight(36);
        connect(voteButton_, &QPushButton::clicked, [this]() { handleVote(); });
        voteLayout->addRow("Candidate:", candidatePicker_);
        voteLayout->addRow(voteButton_);

        layout->addWidget(loginGroup);
        layout->addWidget(voteGroup);
        layout->addStretch();
        return tab;
    }

    QWidget *buildResultsTab() {
        auto *tab = new QWidget();
        auto *layout = new QVBoxLayout(tab);
        layout->setSpacing(14);

        auto *form = new QFormLayout();
        form->setLabelAlignment(Qt::AlignLeft);
        form->setFormAlignment(Qt::AlignTop);
        adminPassword_ = new QLineEdit();
        adminPassword_->setEchoMode(QLineEdit::Password);
        form->addRow("Admin password:", adminPassword_);

        auto *showButton = new QPushButton("Show Results");
        showButton->setMinimumHeight(36);
        connect(showButton, &QPushButton::clicked, [this]() { handleShowResults(); });

        resultsLabel_ = new QLabel();
        resultsLabel_->setText("Results hidden.");
        resultsLabel_->setStyleSheet("color: #374151; font-weight: 600;");
        resultsLabel_->setVisible(false);

        resultsPanel_ = new QGroupBox("Vote Summary");
        resultsPanel_->setVisible(false);
        resultsPanel_->setMaximumHeight(240);
        auto *resultsLayout = new QHBoxLayout(resultsPanel_);
        resultsLayout->setContentsMargins(12, 12, 12, 12);
        resultsLayout->setSpacing(18);

        auto *countsColumn = new QVBoxLayout();
        countsColumn->setSpacing(10);
        for (int i = 0; i < backend::kCandidateCount; ++i) {
            auto *card = new QFrame();
            card->setStyleSheet(
                "QFrame { background: #f8fafc; border: 1px solid #e5e7eb; border-radius: 12px; }"
            );
            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(12, 8, 12, 8);
            auto *nameLabel = new QLabel(QString::fromUtf8(backend::kCandidates[i]));
            nameLabel->setStyleSheet("color: #475569; font-weight: 600;");
            auto *countLabel = new QLabel("0 votes");
            countLabel->setStyleSheet("color: #111827; font-size: 14px; font-weight: 700;");
            countLabels_[i] = countLabel;

            cardLayout->addWidget(nameLabel);
            cardLayout->addStretch();
            cardLayout->addWidget(countLabel);
            countsColumn->addWidget(card);
        }
        countsColumn->addStretch();

        pieChart_ = new PieChartWidget();
        pieChart_->setFixedSize(180, 180);
        pieChart_->setStyleSheet("background: white; border: 1px solid #e5e7eb; border-radius: 12px;");

        legendWidget_ = new QWidget();
        auto *legendLayout = new QVBoxLayout(legendWidget_);
        legendLayout->setContentsMargins(0, 0, 0, 0);
        legendLayout->setSpacing(8);

        const QColor colors[] = {
            QColor("#4f6bed"),
            QColor("#22c55e"),
            QColor("#f59e0b")
        };

        for (int i = 0; i < backend::kCandidateCount; ++i) {
            auto *legendItem = new QWidget();
            auto *legendItemLayout = new QHBoxLayout(legendItem);
            legendItemLayout->setContentsMargins(0, 0, 0, 0);
            legendItemLayout->setSpacing(6);

            auto *colorDot = new QLabel();
            colorDot->setFixedSize(10, 10);
            colorDot->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(colors[i].name()));

            auto *legendLabel = new QLabel(QString::fromUtf8(backend::kCandidates[i]));
            legendLabel->setStyleSheet("color: #475569; font-size: 12px; font-weight: 600;");

            legendItemLayout->addWidget(colorDot);
            legendItemLayout->addWidget(legendLabel);
            legendLayout->addWidget(legendItem);
        }
        legendLayout->addStretch();

        auto *chartRow = new QHBoxLayout();
        chartRow->setSpacing(12);
        chartRow->addWidget(pieChart_, 0, Qt::AlignLeft);
        chartRow->addWidget(legendWidget_, 0, Qt::AlignTop);

        auto *chartContainer = new QWidget();
        chartContainer->setLayout(chartRow);

        resultsLayout->addLayout(countsColumn, 1);
        resultsLayout->addWidget(chartContainer, 0, Qt::AlignRight);

        layout->addLayout(form);
        layout->addWidget(showButton);
        layout->addWidget(resultsLabel_);
        resultsScroll_ = new QScrollArea();
        resultsScroll_->setWidgetResizable(true);
        resultsScroll_->setFrameShape(QFrame::NoFrame);
        resultsScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        resultsScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        resultsScroll_->setStyleSheet(
            "QScrollArea { background: transparent; }"
            "QScrollArea > QWidget > QWidget { background: transparent; }"
            "QScrollBar:vertical { background: transparent; width: 6px; margin: 2px; }"
            "QScrollBar::handle:vertical { background: #c7d2fe; min-height: 24px; border-radius: 3px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: transparent; height: 0px; }"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        );
        resultsScroll_->setWidget(resultsPanel_);
        resultsScroll_->setVisible(false);

        layout->addWidget(resultsScroll_);
        layout->addStretch();
        return tab;
    }

    void handleRegister() {
        if (static_cast<int>(users_.size()) >= backend::kMaxUsers) {
            showMessage("Registration closed", "User limit reached.");
            return;
        }

        std::string cnic = regCnic_->text().toStdString();
        std::string password = regPassword_->text().toStdString();

        if (!backend::IsValidCnic(cnic)) {
            showMessage("Invalid CNIC", "CNIC must be exactly 13 digits.");
            return;
        }

        int existingIndex = -1;
        if (backend::FindUserIndex(users_, cnic, existingIndex)) {
            showMessage("Already registered", "This CNIC is already registered.");
            return;
        }

        backend::User newUser;
        newUser.cnic = cnic;
        newUser.password = backend::HashPassword(password, cnic);
        users_.push_back(newUser);
        backend::SaveData(users_);

        regCnic_->clear();
        regPassword_->clear();
        showMessage("Success", "Registration complete.");
    }

    void handleLogin() {
        std::string cnic = loginCnic_->text().toStdString();
        std::string password = loginPassword_->text().toStdString();

        int index = -1;
        if (!backend::FindUserIndex(users_, cnic, index)) {
            showMessage("Login failed", "User not found.");
            return;
        }

        if (users_[index].password != backend::HashPassword(password, cnic)) {
            showMessage("Login failed", "Invalid password.");
            return;
        }

        loggedInIndex_ = index;
        voteButton_->setEnabled(true);
        showMessage("Login successful", "You can now cast your vote.");
    }

    void handleVote() {
        if (loggedInIndex_ < 0 || loggedInIndex_ >= static_cast<int>(users_.size())) {
            showMessage("Session error", "Please login first.");
            return;
        }

        if (users_[loggedInIndex_].voted) {
            showMessage("Duplicate vote", "You have already voted.");
            return;
        }

        int candidateIndex = candidatePicker_->currentData().toInt();
        users_[loggedInIndex_].voted = true;
        users_[loggedInIndex_].votedFor = candidateIndex;
        voteCounts_[candidateIndex] += 1;
        backend::SaveData(users_);

        showMessage("Vote cast", "Your vote has been recorded.");
    }

    void handleShowResults() {
        std::string adminPassword = adminPassword_->text().toStdString();
        if (adminPassword != backend::kAdminPassword) {
            showMessage("Unauthorized", "Invalid admin password.");
            return;
        }

        std::vector<QString> labels;
        std::vector<int> values;
        for (int i = 0; i < backend::kCandidateCount; ++i) {
            labels.push_back(QString::fromUtf8(backend::kCandidates[i]));
            values.push_back(voteCounts_[i]);
            if (countLabels_[i]) {
                countLabels_[i]->setText(QString("%1 votes").arg(voteCounts_[i]));
            }
        }
        if (pieChart_) {
            pieChart_->setData(labels, values);
        }
        if (resultsPanel_) {
            resultsPanel_->setVisible(true);
        }
        if (resultsScroll_) {
            resultsScroll_->setVisible(true);
        }
    }

    void showMessage(const QString &title, const QString &message) {
        QMessageBox::information(this, title, message);
    }
};
