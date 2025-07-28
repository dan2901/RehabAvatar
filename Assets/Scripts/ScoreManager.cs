using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;
using TMPro;


public class ScoreManager : MonoBehaviour
{
    //we create a public instance of this script
    //so we can access it from other scrips (i.e. collectibleScript)
    public static ScoreManager instance;
    public TextMeshProUGUI scoreText;
    int score = 0;

    private void Awake() {
        instance = this;
    }
    void Start()
    {
        scoreText.text = "Score: " + score.ToString() + " / 10";
    }

    void Update()
    {
        if(score == 10){
            Invoke("SwitchScene",1f);
        }
    }

    public void AddPoint() {
        score += 1;
        scoreText.text = "Score: " + score.ToString() + " / 10";
    }

    private void SwitchScene(){
        SceneManager.LoadScene("End Menu");
    }
}
