#include "Agent.h"
#include <iostream>
#include <random>
#include <algorithm>

Agent::Agent(int n_stat, int n_act, double e, double l, double d, int s, double lam){
    n_states = n_stat;
    n_actions = n_act;
    epsilon = e;
    learning_rate = l;
    discount_rate = d;
    starting_state = s;
    lambda = lam;

    Q = new double[n_states*n_actions];
    V = new double[n_states];
    et = new double[n_states];
    QA = new double[n_states*n_actions];
    QB = new double[n_states*n_actions];
    Q_temperature = new double[n_states*n_actions];
    pii = new double[n_states*n_actions]();
    preferences =  new double[n_states*n_actions]();
    UCB_values = new double[n_states*n_actions];
    nt = new int[n_states*n_actions];
    //dyna_next_state = new int[n_states*n_actions];
    //dyna_reward = new int[n_states*n_actions];

    for (int i=0; i<n_states; i++){
        V[i] = 0;
        et[i] = 0;
        for (int j=0; j<n_actions; j++){
            Q[i*n_actions+j] = 0;
            QA[i*n_actions+j]= 0;
            QB[i*n_actions+j] = 0;
            Q_temperature[i*n_actions+j] = 0;
            UCB_values[i*n_actions+j] = 0;
            nt[i*n_actions+j] = 0;
            //dyna_next_state[i*n_actions+j] = 0;
            //dyna_reward[i*n_act+j] = 0;
        }
    }
};

Agent::~Agent(){
    delete[] Q;
    delete[] V;
    delete[] et;
    delete[] QA;
    delete[] QB;
    delete[] Q_temperature;
    delete[] pii;
    delete[] preferences;
    delete[] UCB_values;
    delete[] nt;
    //delete[] dyna_next_state;
    //delete[] dyna_reward;
};

int Agent::get_initial_state(){
    return starting_state;
};

void Agent::set_initial_state(int s){
    starting_state = s;
};

void Agent::set_epsilon(double e){
    epsilon = e;
};

double* Agent::get_Q(){
    return Q;
};

double* Agent::get_QA(){
    return QA;
};

double* Agent::get_QB(){
    return QB;
};

int Agent::epsilon_greedy(int state, std::vector<int> allowed_actions, int algorithm){
    int act = 0;

    double rand_num = ((double) rand() / (RAND_MAX));
    if (rand_num < epsilon){       //random action
        act = allowed_actions[rand() % allowed_actions.size()];
    } else {                       //greedy action

        int max_idx = allowed_actions[0];
        double max_val = 0;

        if (algorithm == 0 || algorithm == 1 || algorithm == 3) { // SARSA or Q_LEARNING or QV_LEARNING
            max_val = Q[state*n_actions+allowed_actions[0]];

        } else if (algorithm == 2) { // double Q_learning
            max_val = QA[state*n_actions+allowed_actions[0]]+QB[state*n_actions+allowed_actions[0]];
        }

        for (int j=0; j<n_actions; j++){    
            if (std::find(allowed_actions.begin(), allowed_actions.end(), j) != allowed_actions.end()) {
                if (algorithm == 0 || algorithm == 1 || algorithm == 3) { // SARSA or Q_LEARNING or QV_LEARNING
                    if (Q[state*n_actions + j] > max_val){
                        max_val = Q[state*n_actions + j];
                        max_idx = j;
                    }
                }  else if (algorithm == 2) { // double Q_learning
                    if (QA[state*n_actions + j]+QB[state*n_actions + j] > max_val){
                        max_val = QA[state*n_actions + j]+QB[state*n_actions + j];
                        max_idx = j;
                    }
                }
                
            }
        }
        act = max_idx;
    }
    return act;
};

int Agent::boltzmann_exploration(int state, std::vector<int> allowed_actions, int algorithm, double T){

    int act = 0;
    double max_val = -99999;
    double min_val = 999;
    double denom = 0;
    double Qvalue = 0;

    std::vector<double> weights;

    // find max and min
    for (int i=0; i<n_actions; i++){
        if (std::find(allowed_actions.begin(), allowed_actions.end(), i) != allowed_actions.end()) {
            if (algorithm == 2){
                Qvalue = QA[state*n_actions+i];
            } else {
                Qvalue = Q[state*n_actions+i];
            }
            if ( Qvalue > max_val){
                max_val = Qvalue;
            }            
            if (Qvalue < min_val){
                min_val = Qvalue;
            }
        }
    }

    // compute Q/temperatture normalized
    for (int i=0; i<n_actions; i++){
        if (std::find(allowed_actions.begin(), allowed_actions.end(), i) != allowed_actions.end()) {
            if (algorithm == 2){
                Qvalue = QA[state*n_actions+i];
            } else {
                Qvalue = Q[state*n_actions+i];
            }
            Q_temperature[state*n_actions+i] = (Qvalue-min_val)/(max_val-min_val)/T;
            //std::cout<<"Q="<<QA[state*n_actions+i]<<", Q_TEMP="<<Q_temperature[state*n_actions+i]<<std::endl;
        }
    }
    
    for (int i=0; i<n_actions; i++){
        if (std::find(allowed_actions.begin(), allowed_actions.end(), i) != allowed_actions.end()) {
            denom += exp(Q_temperature[state*n_actions+i] - max_val);
            //std::cout<<"Q_TEMP="<<Q_temperature[state*n_actions+i]<<std::endl;
            //std::cout<<"denom="<<denom<<std::endl;
        }
    }

    for (int i=0; i<n_actions; i++){
        if (std::find(allowed_actions.begin(), allowed_actions.end(), i) != allowed_actions.end()) {
            weights.push_back(exp(Q_temperature[state*n_actions+i] - max_val)/denom);
        }
        else {
            weights.push_back(0);
        }
    }

    std::random_device rd;
    std::mt19937 generator(rd());

    //for (std::vector<double>::const_iterator i = weights.begin(); i != weights.end(); ++i)
    //std::cout << *i << ' ';
    //std::cout<<std::endl;


    std::discrete_distribution<int> distribution (weights.begin(), weights.end());
    //generator.seed(time(0));
    act = distribution(generator);

    return act;
};

void Agent::update_avg_reward(int n, double r){
    if (n == 0){
        avg_reward += 1*(r - avg_reward);
    }
    else {
        avg_reward += 1/n*(r - avg_reward);
    }
};

void Agent::update_action_preferences(double r, int state, int act){
    for (int i=0; i<n_actions; i++){
        if (i==act){
            preferences[state*n_actions+i] += learning_rate*(r-avg_reward)*(1-pii[state*n_actions+i]);
        } else {
            preferences[state*n_actions+i] -= learning_rate*(r-avg_reward)*pii[state*n_actions+i];
        }
    }
};

int Agent::UCB(int state, std::vector<int> allowed_actions, int algorithm, int t, double c){
    srand (time(NULL));
    int act = 0;

    for (int j=0; j<n_actions; j++){
        if (std::find(allowed_actions.begin(), allowed_actions.end(), j) != allowed_actions.end()) {
            //std::cout<<"here "<<j<<" is a possible action\n";
            if (nt[state*n_actions+j] != 0){
                if (algorithm == 2){
                    UCB_values[state*n_actions+j] = QA[state*n_actions+j]+QB[state*n_actions+j] + c*sqrt(log(float(t))/nt[state*n_actions+j]);
                } else {
                    //std::cout<<"c="<<c<<", float t"<<float(t)<<", log float t="<<log(float(t))<<", nt="<<nt[state*n_actions+j]<<", c*sqrt(log(float(t))/nt[state*n_actions+j]="<<c*sqrt(log(float(t))/nt[state*n_actions+j])<<std::endl;
                    UCB_values[state*n_actions+j] = Q[state*n_actions+j] + c*sqrt(log(float(t))/nt[state*n_actions+j]);
                }
            }
            else{
                UCB_values[state*n_actions+j] = 10000;
            }
        }
        else{
            UCB_values[state*n_actions+j] = -100000;
        }
    }

    act = choose_max(state);

    nt[state*n_actions+act] += 1;

    return act;
};

int Agent::choose_max(int state){

    std::vector<int> ties;
    double max_value_UCB = -1000;
    srand (time(0));

    for (int i=0; i<n_actions; i++){
        if (UCB_values[state*n_actions+i] > max_value_UCB){
            max_value_UCB = UCB_values[state*n_actions+i];
            ties.clear();
            ties.push_back(i);
        } else if (UCB_values[state*n_actions+i] == max_value_UCB) {
            ties.push_back(i);
        }
    }

    int randomIndex = rand() % ties.size();
    return ties[randomIndex];
};

void Agent::initialize_Q(){
    for (int i=0; i<n_states*n_actions; i++){
        Q[i] = 0;
        Q_temperature[i] = 0;
    }
};

void Agent::initialize_QA_QB(){
    for (int i=0; i<n_states*n_actions; i++){
        QA[i] = 0;
        QB[i] = 0;
    }
};

void Agent::initialize_V(){
    for (int i=0; i<n_states; i++){
        V[i] = 0;
    }
};

void Agent::update_V(int s, double reward, int s_next){
    V[s] += learning_rate*(reward + discount_rate*V[s_next] - V[s]);
};

void Agent::update_Q_SARSA(int s, int a, double reward, int s_next, int a_next){
    Q[s*n_actions+a] += learning_rate*(reward + discount_rate*Q[s_next*n_actions + a_next] - Q[s*n_actions+a]);
};

void Agent::update_Q_Learning(int s, int a, double reward, int s_next, std::vector<int> allowed_actions_s_next) {

	int maximizing_action = allowed_actions_s_next[0];
	double max_val = Q[s_next*n_actions + maximizing_action];

	for (int j = 0; j < n_actions; j++) {
		if (std::find(allowed_actions_s_next.begin(), allowed_actions_s_next.end(), j) != allowed_actions_s_next.end()) {

			if (Q[s_next*n_actions + j] > max_val) {
				max_val = Q[s_next*n_actions + j];
				maximizing_action = j;
			}
		}
	}
	Q[s*n_actions+a] += learning_rate*(reward + discount_rate*Q[s_next*n_actions + maximizing_action] - Q[s*n_actions+a]);
};

void Agent::update_Q_final(int s, int a, double reward){ // both for SARSA and Q learning
    Q[s*n_actions+a] += learning_rate*(reward - Q[s*n_actions+a]);
};

void Agent::update_QA_QB(int s, int a, double reward, int s_next, std::vector<int> allowed_actions, int update_index){

	int maximizing_action = allowed_actions[0];
    double max_val = 0;

    if (update_index == 0){  // 0 == QA
    	max_val = QA[s_next*n_actions + allowed_actions[0]];
    } else if (update_index == 1){  // 1 == QB
	    max_val = QB[s_next*n_actions + allowed_actions[0]];
    }

	for (int j = 0; j < n_actions; j++) {
		if (std::find(allowed_actions.begin(), allowed_actions.end(), j) != allowed_actions.end()) {
            if (update_index == 0){  // 0 == QA
                if (QA[s_next*n_actions + j] > max_val) {
                    max_val = QA[s_next*n_actions + j];
                    maximizing_action = j;
                }
            } else if (update_index == 1){  // 1 == QB
                if (QB[s_next*n_actions + j] > max_val) {
                    max_val = QB[s_next*n_actions + j];
                    maximizing_action = j;
                }
            }
	    }
    }

    if (update_index == 0){  // 1 == QA
	    QA[s*n_actions + a] += learning_rate * (reward + discount_rate*QB[s_next*n_actions + maximizing_action] - QA[s*n_actions + a]);
    } else if (update_index == 1){  // 1 == QB
        QB[s*n_actions + a] += learning_rate * (reward + discount_rate*QA[s_next*n_actions + maximizing_action] - QB[s*n_actions + a]);
    }
};

void Agent::update_QA_QB_final(int s, int a, double reward){
        QA[s*n_actions+a] += learning_rate*(reward - QA[s*n_actions+a]);
        QB[s*n_actions+a] += learning_rate*(reward - QB[s*n_actions+a]);
};

void Agent::update_QV(int s, int a, double reward, int s_new){
    delta = reward + discount_rate*V[s_new] - V[s];
    for (int st=0; st<n_states; st++){
        if (st == s){
            et[s] = discount_rate*lambda*et[s] + 1.0;
        } else {
            et[st] = discount_rate*lambda*et[st];
        }
        V[st] += learning_rate*delta*et[st];
    }
    Q[s*n_actions + a] += learning_rate * (reward + discount_rate*V[s_new] - Q[s*n_actions + a]);
};

void Agent::update_QV_final(int s, int a, double reward){
    delta = reward - V[s];
    et[s] = discount_rate*lambda*et[s] + 1.0; // eta[s];
    V[s] += learning_rate*delta*et[s];
    Q[s*n_actions + a] += learning_rate * (reward - Q[s*n_actions + a]);
};

void Agent::print(double *matrix, int n_rows, int n_cols){
    for (int i=0; i<n_rows; i++){
        std::cout<<"state "<<i<<"  ";
        for (int j=0; j<n_cols; j++){
            std::cout<<matrix[i*n_actions+j]<<"  ";
        }
        std::cout<<std::endl;
    }
};

void Agent::print_nt(){
    for (int i=0; i<n_states; i++){
        std::cout<<"nt["<<i<<"]= ";
        for (int j=0; j<n_actions; j++){
            std::cout<<nt[i*n_actions+j]<<" ";
        }
        std::cout<<std::endl;
    }
};
