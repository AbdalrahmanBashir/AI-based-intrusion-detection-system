
# Intrusion Detection in UAV Networks using Machine Learning Models

## Overview

This project will concentrate on proposing a Machine Learning-based Intrusion Detection System (IDS) to UAV networks. The primary goal of this IDS is to recognize and classify different types of attacks, as well as normal traffic, within a decentralized UAV network. The proposed model leverages several state-of-the-art machine learning approaches. For example:
- Deep Neural Network (DNN)
- Convolutional Neural Network (CNN)
- LightGBM (Gradient Boosted Decision Trees)
- Random Forest
- XGBoost (Extreme Gradient Boosting)

## Project Structure

- **Data Collection**: Network traffic data was collected from UAV simulations using NS-3, capturing features related to traffic behavior and attack patterns.
- **Feature Engineering**: Relevant features were selected and pre-processed to train various machine learning models effectively.
- **Model Training**: Multiple models were trained and tuned to optimize detection accuracy.
- **Evaluation**: Confusion matrices were used to evaluate the performance of each model across different traffic classes, including normal traffic and various types of attacks.

## Attack Types Detected

The IDS was trained to detect the following types of traffic in the UAV network:

1. **AODV Attack**: An attack targeting the AODV (Ad hoc On-demand Distance Vector) routing protocol, causing routing disruption.
2. **Flooding Attack**: An attack where malicious nodes flood the network with excessive traffic to overwhelm legitimate nodes.
3. **Legitimate AODV Traffic**: Normal, non-malicious traffic using the AODV protocol.
4. **Normal Traffic**: Standard network traffic without any attacks.
5. **Sybil Attack**: An attack where a malicious node pretends to have multiple identities to deceive the network.

## Confusion Matrices

The project includes visualizations of confusion matrices for each model, providing insights into their performance:

1. **DNN Confusion Matrix**: Shows the classification performance of the DNN model with high accuracy in detecting most attack types but slight misclassification of AODV attacks as normal traffic.

   ![confusion_matrix_dnn](https://github.com/user-attachments/assets/38f037a9-d8d8-4762-94b2-5175fb177a0e)
   
2. **LightGBM Confusion Matrix**: Highlights the high precision of the LightGBM model, particularly for legitimate AODV traffic, with minimal misclassification.

   ![lightgbm_confusion_matrix](https://github.com/user-attachments/assets/f488689f-3c61-4971-b902-83f60f5eec4c)

3. **CNN Confusion Matrix**: Demonstrates the effectiveness of the CNN model, especially in detecting flooding attacks, with a few misclassifications in Sybil attacks.

   ![normalized_confusion_matrix_cnn_model](https://github.com/user-attachments/assets/5cdcf311-d7aa-4a5b-bec7-e804c9639d82)

4. **Random Forest Confusion Matrix**: The Random Forest model shows strong overall performance, with very few misclassifications in all attack classes.

   ![normalized_confusion_matrix_random_forest](https://github.com/user-attachments/assets/21443cc3-d4a4-4c7c-9c3d-5176616241f3)

5. **XGBoost Confusion Matrix**: The XGBoost model achieves high accuracy across all classes, with minor confusion between AODV and Sybil attacks.

   ![normalized_confusion_matrix_xgboost_model](https://github.com/user-attachments/assets/802bbb66-ddd0-4a90-8826-12b5c56b099e)
   

## Key Findings

- **DNN**: Achieved strong results but showed some confusion between AODV attacks and normal traffic.
- **LightGBM**: Consistently performed well, especially in distinguishing legitimate traffic.
- **CNN**: Excelled in detecting flooding attacks but had minor issues with Sybil attack classification.
- **Random Forest**: Demonstrated high accuracy across all classes with minimal misclassification.
- **XGBoost**: Provided the best overall performance, achieving high detection rates for all attack types.

## Conclusion

This project showcases the comparative effectiveness of different machine learning models in discerning network attacks on UAV networks. The results proved that the performance of ensemble models, namely XGBoost and Random Forest, is good, whereas deep learning models, namely DNN and CNN, perform far better for some specific scenarios. Hence, the findings resulted in the assertion that these models' incorporation in an IDS decentralized model would boost UAV network security relatively concerning common attack types.
## Future Work

- Integrate Federated Learning for decentralized model training.
- Explore additional attack types, including GPS spoofing and jamming.
- Optimize the IDS for deployment on edge devices with limited resources.

## Presentation Link
https://www.youtube.com/watch?v=QkbXri5SZnc
