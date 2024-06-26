@Library('ottopia')_

pipeline {
    agent none

    options {
        timestamps()
        skipDefaultCheckout()
    }

    parameters {
        booleanParam(name: 'WITH_SSH', defaultValue: false, description: 'Do you want SSH?')
    }

    environment { 
        REPO_NAME = 'keepalived-exporter'
        BRANCH_NAME_NO_SLASHES = BRANCH_NAME.replace("/","_")

        AWS_CREDS = 'jenkins-ecr-credentials'
        AWS_ECR_REGION = 'eu-central-1'

        PYPI_SERVER = credentials('jfrog-pypi-server')
    }
    
    stages {
        stage('BuildAndTest') {
                agent {
                    ecs {
                        inheritFrom 'large'
                        image "633878423432.dkr.ecr.eu-central-1.amazonaws.com/jenkins_${REPO_NAME}:x86_64_ubuntu_focal"
                    }
                }
                options {
                    timeout(time: params.WITH_SSH ? 100 : 60, unit: 'MINUTES')
                }
                stages {
                    stage('checkout') {
                        steps {
                            publishChecks name: "${REPO_NAME}", status: 'IN_PROGRESS', summary: 'Summary', text: 'Build is in progress', title: "Jenkins Build ${REPO_NAME}" 
                            gitClone credentials: 'github_ottopia-rnd', branchName: BRANCH_NAME, repoName: REPO_NAME
                        }
                    }
                    stage('Build') {
                        steps {
                            sh '''
                                dpkg-buildpackage -b -uc -ui
                                mv ../*.deb .
                            '''
                        }
                    }
                    stage('deploy to jenkins') {
                        steps {
                            archiveArtifacts artifacts: '*.deb',
                            fingerprint: true,
                            onlyIfSuccessful: true
                        }
                    }
                }
                post {
                    success {
                        publishChecks name: "${REPO_NAME}", summary: 'Summary', text: 'Build succeeded', title: "Jenkins Build ${REPO_NAME}" 
                    }
                    failure {
                        failureInfraFunc repo_name: REPO_NAME, branch_name: BRANCH_NAME, build_number: BUILD_NUMBER
                        publishChecks conclusion: 'FAILURE', name: "${REPO_NAME}", summary: 'Summary', text: 'Build failed', title: "Jenkins Build ${REPO_NAME}" 
                    }
                    aborted {
                        publishChecks conclusion: 'CANCELED', name: "${REPO_NAME}", summary: 'Summary', text: 'Build failed', title: "Jenkins Build ${REPO_NAME}" 
                    }
                    always {
                        script {
                            if (params.WITH_SSH) {
                                enableSSH timeout: 3600
                            }
                        }
                    }
                }
            
        }
    }
}
