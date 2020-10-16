#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <nodes/Node>
#include <QTreeWidgetItem>
#include <QShortcut>
#include <QTimer>
#include <deque>
#include <thread>
#include <mutex>
#include <nodes/DataModelRegistry>

#include "graphic_container.h"
#include "XML_utilities.hpp"
#include "sidepanel_editor.h"
#include "sidepanel_replay.h"
#include "models/SubtreeNodeModel.hpp"

#ifdef ZMQ_FOUND
#include "sidepanel_monitor.h"
#endif

#include <ros/ros.h>
#include <std_msgs/String.h>

namespace Ui {
class MainWindow;
}

namespace QtNodes{
class FlowView;
class FlowScene;
class Node;
}

struct WidgetData {
    WidgetData(QTabWidget* target_widget) {
        this->tabWidget = target_widget;
    }

    AbsBehaviorTree tree;
    QTabWidget* tabWidget;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum SubtreeExpandOption{ SUBTREE_EXPAND,
                              SUBTREE_COLLAPSE,
                              SUBTREE_CHANGE,
                              SUBTREE_REFRESH};

public:
    explicit MainWindow(GraphicMode initial_mode, QWidget *parent = nullptr);
    ~MainWindow() override;

    QDomDocument domFromXML(const QString& xml_text);

    void loadFromXML(const QString &xml_text);

    AbsBehaviorTree newLoadFromXML(const QString &xml_text, const QString &name, WidgetData& widget_data);

    QString saveToXML() const ;

    GraphicContainer* currentTabInfo();

    GraphicContainer *getTabByName(const QString& name);

    void clearTreeModels();

    const NodeModels &registeredModels() const;

    AbsBehaviorTree newOnCreateAbsBehaviorTree(const AbsBehaviorTree &tree, const QString &bt_name, QTabWidget * target_widget);

    void newActionClearTriggered(bool create_new, WidgetData& widget_data);

    GraphicContainer* newTabInfo(WidgetData& widget_data);

    void newOnSceneChanged(WidgetData& widget_data);

    void newOnPushUndo(WidgetData& widget_data);

public slots:

    void onAutoArrange();

    void onSceneChanged();

    void onPushUndo();

    void onUndoInvoked();

    void onRedoInvoked();

    void onConnectionUpdate(bool connected);

    void onRequestSubTreeExpand(GraphicContainer& container,
                                QtNodes::Node& node);

    void onAddToModelRegistry(const NodeModel& model);

    void onDestroySubTree(const QString &ID);

    void onModelRemoveRequested(QString ID);

    virtual void closeEvent(QCloseEvent *event) override;

    void on_actionLoad_triggered();

    void load_two_trees(const QString &left_xml_text, const QString &right_xml_text);

    void load_left_tree(const QString &left_xml_text);

    void load_right_tree(const QString &right_xml_text);

    void on_actionSave_triggered();

    void on_splitter_splitterMoved(int pos = 0, int index = 0);

    void on_toolButtonReorder_pressed();

    void on_toolButtonCenterView_pressed();

    void onCreateAbsBehaviorTree(const AbsBehaviorTree &tree, const QString &bt_name);

    void onChangeNodesStatus(const QString& bt_name, const std::vector<std::pair<int, NodeStatus>>& node_status);

    void on_toolButtonLayout_clicked();

    void on_actionEditor_mode_triggered();

    void on_actionMonitor_mode_triggered();

    void on_actionReplay_mode_triggered();

    void on_tabWidget_currentChanged(int index);

    void onActionClearTriggered(bool create_new);

    void on_actionClear_triggered();

    void onTreeNodeEdited(QString prev_ID, QString new_ID);

    void onTabCustomContextMenuRequested(const QPoint &pos);

    void onTabRenameRequested(int tab_index, QString new_name = QString());

    void onTabSetMainTree(int tab_index);

signals:
    void updateGraphic();

private slots:
    void on_actionAbout_triggered();

    void on_actionReportIssue_triggered();

private:

    void process_hovers();

    void updateCurrentMode();

    void lockEditing(const bool locked);

    bool eventFilter(QObject *obj, QEvent *event) override;

    void resizeEvent(QResizeEvent *) override;

    GraphicContainer *createTab(const QString &name, QTabWidget *tabwidget);

    void refreshNodesLayout(QtNodes::PortLayout new_layout);

    void refreshExpandedSubtrees();

    void streamElementAttributes(QXmlStreamWriter &stream, const QDomElement &element) const;

    QString xmlDocumentToString(const QDomDocument &document) const;

    void recursivelySaveNodeCanonically(QXmlStreamWriter &stream, const QDomNode &parent_node) const;

    struct SavedState
    {
        QString main_tree;
        QString current_tab_name;
        QTransform view_transform;
        QRectF view_area;
        std::map<QString, QByteArray> json_states;
        bool operator ==( const SavedState& other) const;
        bool operator !=( const SavedState& other) const { return !( *this == other); }
    };

    void loadSavedStateFromJson(SavedState state);

    QtNodes::Node *subTreeExpand(GraphicContainer& container,
                       QtNodes::Node &node,
                       SubtreeExpandOption option);

    Ui::MainWindow *ui;

    GraphicMode _current_mode;

    std::shared_ptr<QtNodes::DataModelRegistry> _model_registry;

    std::map<QString, GraphicContainer*> _tab_info;

    std::mutex _mutex;

    std::deque<SavedState> _undo_stack;
    std::deque<SavedState> _redo_stack;
    SavedState _current_state;
    QtNodes::PortLayout _current_layout;

    NodeModels _treenode_models;

    QString _main_tree;

    SidepanelEditor* _editor_widget;
    SidepanelReplay* _replay_widget;
#ifdef ZMQ_FOUND
    SidepanelMonitor* _monitor_widget;
#endif

    MainWindow::SavedState newSaveCurrentState(WidgetData& widget_data);

    MainWindow::SavedState saveCurrentState();

    void clearUndoStacks();

    WidgetData leftData;

    WidgetData rightData;

    QString _left_xml_text;
    QString _right_xml_text;


    void humanTreeCallback(const std_msgs::String::ConstPtr& treeStringMsg);

    void agentTreeCallback(const std_msgs::String::ConstPtr& treeStringMsg);

    ros::Subscriber agent_tree_sub;

    ros::Subscriber human_tree_sub;

    const QString left_tab_name = "Agent";

    const QString right_tab_name = "Human";

    ros::NodeHandle n;
};






#endif // MAINWINDOW_H
