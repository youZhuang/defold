package com.dynamo.cr.web2.client.ui;

import com.dynamo.cr.web2.client.ProjectInfo;
import com.dynamo.cr.web2.client.ProjectInfoList;
import com.google.gwt.core.client.GWT;
import com.google.gwt.core.client.JsArray;
import com.google.gwt.uibinder.client.UiBinder;
import com.google.gwt.uibinder.client.UiField;
import com.google.gwt.user.cellview.client.CellTable;
import com.google.gwt.user.cellview.client.TextColumn;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.Widget;
import com.google.gwt.view.client.ListDataProvider;
import com.google.gwt.view.client.NoSelectionModel;
import com.google.gwt.view.client.SelectionChangeEvent;
import com.google.gwt.view.client.SelectionChangeEvent.Handler;
import com.google.gwt.uibinder.client.UiHandler;
import com.google.gwt.event.dom.client.ClickEvent;

public class DashboardView extends Composite {

    public interface Presenter {

        void showProject(ProjectInfo projectInfo);

        void onNewProject();

    }

    private static DashboardUiBinder uiBinder = GWT
            .create(DashboardUiBinder.class);
    @UiField CellTable<ProjectInfo> projectsTable;
    @UiField Button newProjectButton;

    interface DashboardUiBinder extends UiBinder<Widget, DashboardView> {
    }

    private ListDataProvider<ProjectInfo> projectTableDataProvider;
    private Presenter listener;

    public DashboardView() {
        initWidget(uiBinder.createAndBindUi(this));
        TextColumn<ProjectInfo> nameColumn = new TextColumn<ProjectInfo>() {
            @Override
            public String getValue(ProjectInfo projectInfo) {
              return projectInfo.getName();
            }
        };

        TextColumn<ProjectInfo> descriptionColumn = new TextColumn<ProjectInfo>() {
            @Override
            public String getValue(ProjectInfo projectInfo) {
              return projectInfo.getDescription();
            }
        };

        TextColumn<ProjectInfo> ownerColumn = new TextColumn<ProjectInfo>() {
            @Override
            public String getValue(ProjectInfo projectInfo) {
              return projectInfo.getOwner().getEmail();
            }
        };

        projectsTable.addColumn(nameColumn, "Name");
        projectsTable.addColumn(descriptionColumn, "Description");
        projectsTable.addColumn(ownerColumn, "Owner");

        projectTableDataProvider = new ListDataProvider<ProjectInfo>();
        projectTableDataProvider.addDataDisplay(projectsTable);

        final NoSelectionModel<ProjectInfo> selectionModel = new NoSelectionModel<ProjectInfo>();
        projectsTable.setSelectionModel(selectionModel);
        projectsTable.getSelectionModel().addSelectionChangeHandler(new Handler() {

            @Override
            public void onSelectionChange(SelectionChangeEvent event) {
                ProjectInfo projectInfo = selectionModel.getLastSelectedObject();
                listener.showProject(projectInfo);
                //projectDetails.setProjectInfo(projectInfo);
                //deckPanel.showWidget(1);
            }
        });
    }

    public void setProjectInfoList(ProjectInfoList projectInfoList) {
        projectTableDataProvider.getList().clear();
        JsArray<ProjectInfo> projects = projectInfoList.getProjects();
        for (int i = 0; i < projects.length(); ++i) {
            projectTableDataProvider.getList().add(projects.get(i));
        }
    }

    public void clearProjectInfoList() {
        projectTableDataProvider.getList().clear();
    }

    public void setPresenter(DashboardView.Presenter listener) {
        this.listener = listener;
    }

    @UiHandler("newProjectButton")
    void onNewProjectButtonClick(ClickEvent event) {
        this.listener.onNewProject();
    }
}
