using ASPA0010_1.DTO;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

namespace ASPA0010_1.Controllers
{
    [Route("api/Results")]
    [ApiController]
    public class ResultsController : ControllerBase
    {

        private readonly ResultsCollection.Results _results;
        private readonly ResultsAuthenticate.Authenticate _auth;

        public ResultsController(ResultsCollection.Results res, ResultsAuthenticate.Authenticate auth)
        {
            _results = res;
            _auth = auth;
        }

        [HttpPost("SignIn")]
        public async Task<IActionResult> SignIn([FromBody] LoginModel model)
        {
            if (model == null || string.IsNullOrEmpty(model.login) || string.IsNullOrEmpty(model.password))
                return BadRequest();

            bool isSuccess = await _auth.SignInAsync(model.login, model.password);

            if (isSuccess)
                return Ok("Cookie-токен установлен");

            return NotFound();
        }

        [HttpGet("SignOut")]
        public async Task<IActionResult> SignOut()
        {
            string shitJwt = await _auth.SignOutAsync();
            return Ok(new { token = shitJwt });
        }

        [HttpGet]
        [Authorize(Roles ="READER")]
        public IActionResult GetResults()
        {
            var ris = _results.GetAll();

            if (ris.Count!=0)
            {
                return Ok(ris);
            }
            else
            {
                return NotFound();
            }
            
        }

        [HttpGet("{key}")]
        [Authorize(Roles = "READER")]
        public IActionResult GetDyId(int key)
        {
            var ris = _results.GetById(key);

            if (ris != null)
            {
                return Ok(ris);
            }
            else
            {
                return NotFound();
            }
        }

        [HttpPost]
        [Authorize(Roles = "WRITER")]
        public IActionResult AddResults(string value)
        {
            var ris = _results.Add(value);
            return Created();
        }

        [HttpPut("{key}")]
        [Authorize(Roles = "WRITER")]
        public IActionResult ChangeResult(int key, string value)
        {
            bool check = _results.Change(key, value);
            if(check)
            {
                return Ok(check);
            }
            return BadRequest();
        }

        [HttpDelete("{key}")]
        [Authorize(Roles = "WRITER")]
        public IActionResult DeleteResult(int key)
        {
            bool check = _results.DeleteById(key);
            return check ? Ok() : BadRequest();
        }

    }
}
