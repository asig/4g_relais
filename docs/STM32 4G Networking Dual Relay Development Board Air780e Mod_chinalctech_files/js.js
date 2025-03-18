$(document).ready(function() {
	var flbox = $('.sechan');
    nav = $('.nav');
    box = $('#container');
    flbox.click(function() {
			nav.animate({right:'0px'},'500');
			$('.nav').css("display", "block");
			$('.mainboxbg').css("display", "block");
			 return; 

    });
	$('.mainboxbg').click(function(){
		nav.animate({right:'-40%'},'500');
		$(this).hide();
       return; 
      }	)
  $('.spanfl').click( function(){
    $('.flboxsub').show();
  });
   $('.flcolose').click( function(){
    $('.flboxsub').hide();
  })
	
});
function autoh(){
      var ath=$('.autoh').height();
      $('.autohh').css({'height':ath-20+'px'});
    }
function autoimg_list(){
        var wit=$('.imgautolist li img').width(); 
        var cplbinfo_h=$(".imgautolist li .cplbinfo").height();
        var nnh = cplbinfo_h +  wit;
        $('.imgautolist').find("li .imgbox").css({'height':wit+'px'});
        // $('.imgautolist').find("li").css({'height':nnh+20+'px'});      
        // $('.imgautolist').find("li a").css({'height':nnh-1+'px'});
}